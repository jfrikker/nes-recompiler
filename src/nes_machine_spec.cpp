#include "nes_machine_spec.hpp"

#include <cstring>

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
using llvm::Type;
using llvm::ArrayType;
using llvm::GlobalVariable;
using llvm::GlobalValue;
using llvm::Module;
using llvm::IRBuilder;
using llvm::Value;
using llvm::getGlobalContext;
using llvm::Constant;
using llvm::APInt;

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
  new GlobalVariable(modgen.getModule(), ArrayType::get(modgen.getWordType(), 65536), false, GlobalValue::PrivateLinkage, NULL, "ram");
}

Value *NesMachineSpec::generateLoad(addr address, BlockGenerator &blockgen) const {
  IRBuilder<> &builder = blockgen.getBuilder();
  Value *ram = blockgen.getModule().getGlobalVariable("ram", true);
  Value *offset = blockgen.getConstant(address);
  Value *ptr = builder.CreateExtractElement(ram, offset);
  return builder.CreateLoad(ptr);
}
