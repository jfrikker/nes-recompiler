#pragma once

#include <iostream>
#include <llvm/IR/IRBuilder.h>

#include "memory.hpp"
#include "codegen.hpp"

class MachineSpec;

class Instruction {
  public:
    Instruction(addr, const char *);

    virtual std::ostream &write(std::ostream& o) const = 0;
    virtual addr getEncodedLength() const = 0;
    virtual bool isTerminal() const;
    virtual bool isBranch() const;
    virtual addr getBranchTarget() const;
    virtual void generateCode(BlockGenerator &codegen) const;

    addr getFollowingLocation() const;

  protected:
    const char *opcode;
    addr location;
};

std::ostream &operator<<(std::ostream &o, const Instruction &instruction);

Instruction *readInstruction(addr, const MachineSpec &);
