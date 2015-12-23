#include "nes_machine_spec.hpp"

#include <cstring>

#include <vector>
using std::vector;

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
using llvm::Type;
using llvm::ArrayType;
using llvm::FunctionType;
using llvm::GlobalVariable;
using llvm::GlobalValue;
using llvm::Module;
using llvm::IRBuilder;
using llvm::Value;
using llvm::getGlobalContext;
using llvm::Constant;
using llvm::APInt;
using llvm::ConstantAggregateZero;
using llvm::ArrayRef;
using llvm::Function;

const char *NES_IDENTIFIER = "NES\x1a";

typedef struct {
  char identifier[4];
  uint8_t prgRomSize;
  uint8_t chrRomSize;
  uint8_t flags1;
  uint8_t flags2;
  uint8_t ramSize;
  uint8_t flags3;
  uint8_t flags4;
  char zero[5];
} NESHeader;

bool isValidHeader(const NESHeader *header) {
  return !strncmp(header->identifier, NES_IDENTIFIER, 4);
}

NesMachineSpec *loadNesMachine(const word *buffer) {
  NESHeader *header = (NESHeader *)buffer;
  if (!isValidHeader(header)) {
    return NULL;
  }

  NesMachineSpec *result = new NesMachineSpec();
  buffer += sizeof(NESHeader);

  result->prgRomSize = header->prgRomSize * 16384;
  result->prgRomOffset = ADDR_MAX - result->prgRomSize + 1;
  result->prgRom = buffer;

  return result;
}

addr NesMachineSpec::getPrgRomSize() const {
  return prgRomSize;
}

addr NesMachineSpec::getPrgRomOffset() const {
  return prgRomOffset;
}

const word *NesMachineSpec::getPrgRom() const {
  return prgRom;
}

word NesMachineSpec::readWord(addr address) const {
  if (address >= prgRomOffset) {
    return prgRom[address - prgRomOffset];
  }

  return 0;
}

void NesMachineSpec::writeLLVMHeader(ModuleGenerator &modgen) const {
  ArrayType *ramType = ArrayType::get(modgen.getWordType(), 65536);
  GlobalVariable *ram = new GlobalVariable(modgen.getModule(), ramType, false, GlobalValue::ExternalLinkage, NULL, "ram");
  ConstantAggregateZero* ramInit = ConstantAggregateZero::get(ramType);
  ram->setInitializer(ramInit);

  vector<Type *> args;
  args.push_back(modgen.getWordType());
  FunctionType *wfType = FunctionType::get(Type::getVoidTy(getGlobalContext()), args, false);

  args.clear();
  FunctionType *rfType = FunctionType::get(modgen.getWordType(), args, false);

  Function::Create(rfType, Function::ExternalLinkage, "readPPUStatus", &(modgen.getModule()));

  Function::Create(wfType, Function::ExternalLinkage, "writePPUScroll", &(modgen.getModule()));
  Function::Create(wfType, Function::ExternalLinkage, "writePPUCtrl", &(modgen.getModule()));
  Function::Create(wfType, Function::ExternalLinkage, "writePPUAddr", &(modgen.getModule()));
  Function::Create(wfType, Function::ExternalLinkage, "writePPUData", &(modgen.getModule()));
}

Value *callReadFunc(const char *name, BlockGenerator &blockgen) {
  Function *func = blockgen.getModule().getFunction(name);
  blockgen.getBuilder().CreateCall(func, ArrayRef<Value *>());
}

Value *NesMachineSpec::generateLoad(addr address, BlockGenerator &blockgen) const {
  switch(address) {
    case 0x2002:
      return callReadFunc("readPPUStatus", blockgen);
    default:
      IRBuilder<> &builder = blockgen.getBuilder();
      Value *ram = blockgen.getModule().getGlobalVariable("ram", false);
      Value *offset = blockgen.getConstant(address);
      Value *indexList[2] = {blockgen.getConstant((addr)0), offset};
      Value *ptr = builder.CreateGEP(ram, ArrayRef<Value *>(indexList, 2));
      return builder.CreateLoad(ptr);
  };
}

Value *NesMachineSpec::generateLoad(Value *address, BlockGenerator &blockgen) const {
  // TODO
  IRBuilder<> &builder = blockgen.getBuilder();
  Value *ram = blockgen.getModule().getGlobalVariable("ram", false);
  Value *indexList[2] = {blockgen.getConstant((addr)0), address};
  Value *ptr = builder.CreateGEP(ram, ArrayRef<Value *>(indexList, 2));
  return builder.CreateLoad(ptr);
}

void callStoreFunc(const char *name, Value *value, BlockGenerator &blockgen) {
  Function *func = blockgen.getModule().getFunction(name);
  blockgen.getBuilder().CreateCall(func, ArrayRef<Value *>(&value, 1));
}

void NesMachineSpec::generateStore(addr address, Value *value, BlockGenerator &blockgen) const {
  switch(address) {
    case 0x2000:
      callStoreFunc("writePPUCtrl", value, blockgen);
      break;
    case 0x2005:
      callStoreFunc("writePPUScroll", value, blockgen);
      break;
    case 0x2006:
      callStoreFunc("writePPUAddr", value, blockgen);
      break;
    case 0x2007:
      callStoreFunc("writePPUData", value, blockgen);
      break;
    default:
      IRBuilder<> &builder = blockgen.getBuilder();
      Value *ram = blockgen.getModule().getGlobalVariable("ram", true);
      Value *offset = blockgen.getConstant(address);
      Value *indexList[2] = {blockgen.getConstant((addr)0), offset};
      Value *ptr = builder.CreateGEP(ram, ArrayRef<Value *>(indexList, 2));
      builder.CreateStore(value, ptr);
  };
}

void NesMachineSpec::generateStore(Value *address, llvm::Value *value, BlockGenerator &blockgen) const {
  // TODO
  IRBuilder<> &builder = blockgen.getBuilder();
  Value *ram = blockgen.getModule().getGlobalVariable("ram", true);
  Value *indexList[2] = {blockgen.getConstant((addr)0), address};
  Value *ptr = builder.CreateGEP(ram, ArrayRef<Value *>(indexList, 2));
  builder.CreateStore(value, ptr);
}
