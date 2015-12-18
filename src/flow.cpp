#include "flow.hpp"

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

void writeBlock(addr start, addr end, BasicBlock *block, ModuleGenerator &modgen) {
  BlockGenerator blockgen(modgen, block);
  while (start < end) {
    Instruction *inst = readInstruction(start, blockgen.getMachine());
    inst->generateCode(blockgen);
    start = inst->getFollowingLocation();
    delete inst;
  }
}

void writeFunction(addr start, ModuleGenerator &modgen) {
  set<addr> insts;
  identifyFunction(start, modgen.getMachine(), insts);

  set<addr> blocks;
  identifyBlocks(start, insts, modgen.getMachine(), blocks);

  Function *func = writeFunctionDecl(start, modgen);

  BasicBlock *body = BasicBlock::Create(getGlobalContext(), "start", func);

  set<addr>::iterator it = blocks.begin();
  addr blockStart = *it;
  addr blockEnd = *(++it);
  writeBlock(blockStart, blockEnd, body, modgen);
}
