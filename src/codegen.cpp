#include "codegen.hpp"

#include <map>
using std::map;

using llvm::Module;
using llvm::IRBuilder;
using llvm::Type;
using llvm::StructType;
using llvm::Value;
using llvm::APInt;
using llvm::Constant;
using llvm::Function;
using llvm::BasicBlock;
using llvm::ArrayRef;
using llvm::getGlobalContext;

ModuleGenerator::ModuleGenerator(const char *moduleName, const MachineSpec &machine) : 
machine (machine),
module(moduleName, getGlobalContext())
{
  Type *fieldTypes[] = {getWordType(), getWordType(), getWordType(), getFlagType(), getFlagType(), getFlagType(), getFlagType()};
  regStructType = StructType::create(ArrayRef<Type *>(fieldTypes, 7));
}

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

Type *ModuleGenerator::getFlagType() const {
  return Type::getInt1Ty(getGlobalContext());
}

StructType *ModuleGenerator::getRegStructType() const {
  return regStructType;
}

Value *ModuleGenerator::getConstant(word val) const {
  return Constant::getIntegerValue(getWordType(), APInt(8, val));
}

Value *ModuleGenerator::getConstant(addr val) const {
  return Constant::getIntegerValue(getAddrType(), APInt(16, val));
}

BlockGenerator::BlockGenerator(ModuleGenerator &moduleGenerator, BasicBlock *block, map<addr, BlockGenerator *> &blocks) : 
  modgen(moduleGenerator),
  builder(block),
  blocks(blocks) {
  phis[REG_A] = builder.CreatePHI(getWordType(), 0);
  setRegValue(REG_A, phis[REG_A]);

  phis[REG_X] = builder.CreatePHI(getWordType(), 0);
  setRegValue(REG_X, phis[REG_X]);

  phis[REG_Y] = builder.CreatePHI(getWordType(), 0);
  setRegValue(REG_Y, phis[REG_Y]);

  phis[REG_N] = builder.CreatePHI(getFlagType(), 0);
  setRegValue(REG_N, phis[REG_N]);

  phis[REG_V] = builder.CreatePHI(getFlagType(), 0);
  setRegValue(REG_V, phis[REG_V]);

  phis[REG_Z] = builder.CreatePHI(getFlagType(), 0);
  setRegValue(REG_Z, phis[REG_Z]);

  phis[REG_C] = builder.CreatePHI(getFlagType(), 0);
  setRegValue(REG_C, phis[REG_C]);
}

Module &BlockGenerator::getModule() {
  return modgen.getModule();
}

const MachineSpec &BlockGenerator::getMachine() const {
  return modgen.getMachine();
}

BasicBlock *BlockGenerator::getBlock() {
  return builder.GetInsertBlock();
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

Type *BlockGenerator::getFlagType() const {
  return modgen.getFlagType();
}

StructType *BlockGenerator::getRegStructType() const {
  return modgen.getRegStructType();
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

void BlockGenerator::addIncomingValue(Register reg, Value *val, BasicBlock *block) {
  phis[reg]->addIncoming(val, block);
}

void BlockGenerator::addIncomingValues(BlockGenerator &blockgen) {
  BasicBlock *myBlock = getBlock();
  blockgen.addIncomingValue(REG_A, getRegValue(REG_A), myBlock);
  blockgen.addIncomingValue(REG_X, getRegValue(REG_X), myBlock);
  blockgen.addIncomingValue(REG_Y, getRegValue(REG_Y), myBlock);
  blockgen.addIncomingValue(REG_N, getRegValue(REG_N), myBlock);
  blockgen.addIncomingValue(REG_V, getRegValue(REG_V), myBlock);
  blockgen.addIncomingValue(REG_Z, getRegValue(REG_Z), myBlock);
  blockgen.addIncomingValue(REG_C, getRegValue(REG_C), myBlock);
}

void BlockGenerator::addIncomingValues(addr blockStart) {
  addIncomingValues(*(blocks[blockStart]));
}

void BlockGenerator::generateJump(addr targetBlock) {
  BlockGenerator *target = blocks[targetBlock];
  builder.CreateBr(target->getBlock());
  addIncomingValues(*target);
}

void BlockGenerator::generateConditionalJump(Value *condition, addr trueBlock, addr falseBlock) {
  BlockGenerator *trueGen = blocks[trueBlock];
  BlockGenerator *falseGen = blocks[falseBlock];
  builder.CreateCondBr(condition, trueGen->getBlock(), falseGen->getBlock());
  addIncomingValues(*trueGen);
  addIncomingValues(*falseGen);
}
