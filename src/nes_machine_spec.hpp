#pragma once

#include "machine_spec.hpp"
#include "memory.hpp"

class NesMachineSpec : public MachineSpec {
  friend NesMachineSpec *loadNesMachine(const word *buffer);

  public:
    addr getPrgRomSize() const;
    addr getPrgRomOffset() const;
    const word *getPrgRom() const;

  public:
    virtual word readWord(addr) const;
    virtual void writeLLVMHeader(ModuleGenerator &modgen) const;
    virtual llvm::Value *generateLoad(addr address, BlockGenerator &blockgen) const;
    virtual void generateStore(addr address, llvm::Value *value, BlockGenerator &blockgen) const;

  private:
    addr prgRomOffset;
    addr prgRomSize;
    const word *prgRom;
};

NesMachineSpec *loadNesMachine(const word *buffer);
