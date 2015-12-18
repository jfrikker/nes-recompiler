#pragma once

#include <map>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"

#include "memory.hpp"

class MachineSpec;

class ModuleGenerator {
  public:
    ModuleGenerator(const char *moduleName, const MachineSpec &machine);

    llvm::Module &getModule();
    const MachineSpec &getMachine() const;
    void write() const;
    llvm::Type *getWordType() const;
    llvm::Type *getAddrType() const;
    llvm::Value *getConstant(word val) const;
    llvm::Value *getConstant(addr val) const;

  private:
    llvm::Module module;
    const MachineSpec &machine;
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
    BlockGenerator(ModuleGenerator &moduleGenerator, llvm::BasicBlock *block);

    llvm::Module &getModule();
    const MachineSpec &getMachine() const;
    llvm::IRBuilder<> &getBuilder();
    llvm::Type *getWordType() const;
    llvm::Type *getAddrType() const;
    llvm::Value *getConstant(word val) const;
    llvm::Value *getConstant(addr val) const;
    llvm::Value *getRegValue(Register);
    void setRegValue(Register reg, llvm::Value *val);

  private:
    llvm::IRBuilder<> builder;
    std::map<Register, llvm::Value *> values;
    ModuleGenerator &modgen;
};
