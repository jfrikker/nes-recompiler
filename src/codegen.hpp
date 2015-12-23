#pragma once

#include <map>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"

#include "memory.hpp"

class MachineSpec;
class Instruction;

class ModuleGenerator {
  public:
    ModuleGenerator(const char *moduleName, const MachineSpec &machine);

    llvm::Module &getModule();
    const MachineSpec &getMachine() const;
    void write() const;
    llvm::Type *getWordType() const;
    llvm::Type *getAddrType() const;
    llvm::Type *getFlagType() const;
    llvm::StructType *getRegStructType() const;
    llvm::Value *getConstant(word val) const;
    llvm::Value *getConstant(addr val) const;

  private:
    llvm::Module module;
    const MachineSpec &machine;
    llvm::StructType *regStructType;
};

enum Register {
  REG_A,
  REG_X,
  REG_Y,
  REG_N,
  REG_V,
  REG_Z,
  REG_C
};

class BlockGenerator {
  public:
    BlockGenerator(ModuleGenerator &moduleGenerator, llvm::BasicBlock *block, std::map<addr, BlockGenerator *> &blocks);

    llvm::Module &getModule();
    const MachineSpec &getMachine() const;
    llvm::IRBuilder<> &getBuilder();
    llvm::BasicBlock *getBlock();
    llvm::Type *getWordType() const;
    llvm::Type *getAddrType() const;
    llvm::Type *getFlagType() const;
    llvm::StructType *getRegStructType() const;
    llvm::Value *getConstant(word val) const;
    llvm::Value *getConstant(addr val) const;
    llvm::Value *getRegValue(Register);
    void setRegValue(Register reg, llvm::Value *val);
    void addIncomingValue(Register reg, llvm::Value *val, llvm::BasicBlock *block);
    void addIncomingValues(BlockGenerator &blockgen);
    void addIncomingValues(addr blockStart);
    void generateJump(addr targetBlock);
    void generateConditionalJump(llvm::Value *condition, addr trueBlock, addr falseBlock);

  private:
    llvm::IRBuilder<> builder;
    std::map<Register, llvm::Value *> values;
    ModuleGenerator &modgen;
    std::map<addr, BlockGenerator *> &blocks;
    std::map<Register, llvm::PHINode *> phis;
};
