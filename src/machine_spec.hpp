#pragma once

#include "memory.hpp"
#include "codegen.hpp"

class MachineSpec {
  public:
    virtual word readWord(addr) const = 0;
    addr readAddr(addr) const;

    addr getNMIAddr() const;
    addr getRSTAddr() const;
    addr getBRKAddr() const;

    virtual void writeLLVMHeader(ModuleGenerator &modgen) const = 0;
    virtual llvm::Value *generateLoad(addr address, BlockGenerator &blockgen) const = 0;
};
