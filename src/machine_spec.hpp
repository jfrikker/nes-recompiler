#pragma once

#include <llvm/IR/Value.h>

#include "memory.hpp"

class ModuleGenerator;
class BlockGenerator;

class MachineSpec {
  public:
    virtual word readWord(addr) const = 0;
    addr readAddr(addr) const;

    addr getNMIAddr() const;
    addr getRSTAddr() const;
    addr getBRKAddr() const;

    virtual void writeLLVMHeader(ModuleGenerator &modgen) const = 0;
    virtual llvm::Value *generateLoad(addr address, BlockGenerator &blockgen) const = 0;
    virtual llvm::Value *generateLoad(llvm::Value *address, BlockGenerator &blockgen) const = 0;
    virtual void generateStore(addr address, llvm::Value *value, BlockGenerator &blockgen) const = 0;
    virtual void generateStore(llvm::Value *address, llvm::Value *value, BlockGenerator &blockgen) const = 0;
};
