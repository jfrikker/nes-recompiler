#include "codegen.hpp"

using llvm::Module;
using llvm::IRBuilder;
using llvm::Type;
using llvm::Value;
using llvm::APInt;
using llvm::Constant;
using llvm::Function;
using llvm::BasicBlock;
using llvm::getGlobalContext;

ModuleGenerator::ModuleGenerator(const char *moduleName, const MachineSpec &machine) : 
machine (machine),
module(moduleName, getGlobalContext())
{ }

Module &ModuleGenerator::getModule() {
  return module;
}

const MachineSpec &ModuleGenerator::getMachine() const {
  return machine;
}

void ModuleGenerator::write() const {
  module.dump();
}

Type *ModuleGenerator::getWordType() const {
  return Type::getInt8Ty(getGlobalContext());
}

Type *ModuleGenerator::getAddrType() const {
  return Type::getInt16Ty(getGlobalContext());
}

Value *ModuleGenerator::getConstant(word val) const {
  return Constant::getIntegerValue(getWordType(), APInt(8, val));
}

Value *ModuleGenerator::getConstant(addr val) const {
  return Constant::getIntegerValue(getAddrType(), APInt(16, val));
}

BlockGenerator::BlockGenerator(ModuleGenerator &moduleGenerator, BasicBlock *block) : 
  modgen(moduleGenerator),
  builder(block) { }

Module &BlockGenerator::getModule() {
  return modgen.getModule();
}

const MachineSpec &BlockGenerator::getMachine() const {
  return modgen.getMachine();
}

IRBuilder<> &BlockGenerator::getBuilder() {
  return builder;
}

Type *BlockGenerator::getWordType() const {
  return modgen.getWordType();
}

Type *BlockGenerator::getAddrType() const {
  return modgen.getAddrType();
}

Value *BlockGenerator::getConstant(word val) const {
  return modgen.getConstant(val);
}

Value *BlockGenerator::getConstant(addr val) const {
  return modgen.getConstant(val);
}

Value *BlockGenerator::getRegValue(Register reg) {
  return values[reg];
}

void BlockGenerator::setRegValue(Register reg, Value *val) {
  values[reg] = val;
}
