#include "flow.hpp"

#include <map>
using std::map;

#include <set>
using std::set;

#include <stack>
using std::stack;

#include <memory>
using std::unique_ptr;

#include <vector>
using std::vector;

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
using llvm::Module;
using llvm::Type;
using llvm::FunctionType;
using llvm::Function;
using llvm::getGlobalContext;
using llvm::BasicBlock;
using llvm::IRBuilder;

#include "instruction.hpp"

#include <iostream>

#include <iomanip>
using std::hex;
using std::uppercase;
using std::setfill;
using std::setw;

#include <sstream>
using std::stringstream;

void identifyFunction(addr start, const MachineSpec &machine, set<addr> &out) {
  stack<addr> remaining;
  remaining.push(start);

  while (!remaining.empty()) {
    addr address = remaining.top();
    remaining.pop();
    if (!out.insert(address).second) {
      continue;
    }

    unique_ptr<Instruction> instruction(readInstruction(address, machine));

    if (instruction->isTerminal()) {
      continue;
    }

    if (instruction->isBranch()) {
      remaining.push(instruction->getBranchTarget());
    }

    remaining.push(instruction->getFollowingLocation());
  }
}

void identifyBlocks(addr start, const set<addr> &function, const MachineSpec &machine, set<addr> &out) {
  out.insert(start);
  for (set<addr>::iterator it = function.begin(); it != function.end(); it++) {
    unique_ptr<Instruction> instruction(readInstruction(*it, machine));
    if (instruction->isBranch()) {
      out.insert(instruction->getFollowingLocation());
      out.insert(instruction->getBranchTarget());
    }
  }
}

Function *writeFunctionDecl(addr start, ModuleGenerator &modgen) {
  char name[7];
  sprintf(name, "f_%04X", start);

  vector<Type *> args;
  args.push_back(Type::getInt8Ty(getGlobalContext()));
  args.push_back(Type::getInt8Ty(getGlobalContext()));
  args.push_back(Type::getInt8Ty(getGlobalContext()));
  args.push_back(Type::getInt1Ty(getGlobalContext()));
  args.push_back(Type::getInt1Ty(getGlobalContext()));
  args.push_back(Type::getInt1Ty(getGlobalContext()));
  args.push_back(Type::getInt1Ty(getGlobalContext()));
  FunctionType *ft = FunctionType::get(Type::getVoidTy(getGlobalContext()), args, false);
  Function *func = Function::Create(ft, Function::ExternalLinkage, name, &(modgen.getModule()));

  const char *argName[] = {"A", "X", "Y", "N", "V", "Z", "C"};

  int i = 0;
  for (auto &arg : func->args()) {
    arg.setName(argName[i++]);
  }

  return func;
}

void writeBlock(addr start, addr end, BlockGenerator &blockgen) {
  std::unique_ptr<Instruction> lastInstruction;

  while (start < end) {
    lastInstruction.reset(readInstruction(start, blockgen.getMachine()));
    lastInstruction->generateCode(blockgen);
    start = lastInstruction->getFollowingLocation();
  }

  if (!lastInstruction->isBranch() && !lastInstruction->isTerminal()) {
    blockgen.generateJump(lastInstruction->getFollowingLocation());
  }
}

void writeFunction(addr start, ModuleGenerator &modgen) {
  set<addr> insts;
  identifyFunction(start, modgen.getMachine(), insts);

  set<addr> blocks;
  identifyBlocks(start, insts, modgen.getMachine(), blocks);

  Function *func = writeFunctionDecl(start, modgen);

  BasicBlock *startBlock = BasicBlock::Create(getGlobalContext(), "start", func);

  map<addr, BlockGenerator *> blockMap;
  for (auto &blockStart : blocks) {
    stringstream name;
    name << "l_" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << blockStart;
    BasicBlock *block = BasicBlock::Create(getGlobalContext(), name.str(), func);
    blockMap[blockStart] = new BlockGenerator(modgen, block, blockMap);
  }

  addr previous = 0;
  for (auto &blockStart : blocks) {
    if (previous != 0) {
      writeBlock(previous, blockStart, *(blockMap[previous]));
    }
    previous = blockStart;
  }

  Register argRegs[] = {REG_A, REG_X, REG_Y, REG_N, REG_V, REG_Z, REG_C};

  int i = 0;
  for (auto &arg : func->args()) {
    blockMap[start]->addIncomingValue(argRegs[i++], &arg, startBlock);
  }

  IRBuilder<> builder(startBlock);
  builder.CreateBr(blockMap[start]->getBlock());
}
