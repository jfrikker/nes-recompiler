#include "instruction.hpp"

#include <iomanip>
using std::hex;
using std::uppercase;
using std::setfill;
using std::setw;

#include <iostream>
using std::ostream;

using llvm::Value;
using llvm::IRBuilder;

#include "machine_spec.hpp"

#define DEF_NO_ARG_INST(OPCODE) class OPCODE : public NoArgInstruction { \
  public: \
    OPCODE(addr location) : NoArgInstruction(location, #OPCODE) {}

#define DEF_WORD_ARG_INST(OPCODE) class OPCODE : public ArgInstruction { \
  public: \
    OPCODE(addr location, Argument *argument) : \
      ArgInstruction(location, #OPCODE, argument) {}

#define DEF_ADDR_ARG_INST(OPCODE) DEF_WORD_ARG_INST(OPCODE)

#define DEF_BRANCH virtual bool isBranch() const { \
  return true; \
} \
virtual addr getBranchTarget() const { \
  return arg->getAddrArg(location + getEncodedLength()); \
}

ostream &operator<<(ostream &o, const Instruction &instruction) {
  return instruction.write(o);
}

class Argument {
  public:
    virtual ostream &write(ostream& o) const = 0;
    virtual addr getEncodedLength() const = 0;
    virtual addr getAddrArg(addr instructionAddr) const {
      return 0;
    };
    virtual Value *getWordArgExpr(addr instructionAddr, BlockGenerator &blockgen) const {
      return blockgen.getConstant((word)0);
    }
    virtual bool isAbsolute() const {
      return false;
    }
};

ostream &operator<<(ostream &o, const Argument &mode) {
  return mode.write(o);
}

class ABSArgument : public Argument {
  public:
    ABSArgument(addr address, const MachineSpec &machine) {
      this->address = machine.readAddr(address);
    }

    ostream &write(ostream& o) const {
      return o << "$" << hex << uppercase << setfill('0') << setw(4) << address;
    }

    addr getEncodedLength() const {
      return 2;
    }

    virtual addr getAddrArg(addr instructionAddr) const {
      return address;
    }

    virtual bool isAbsolute() const {
      return true;
    }

  private:
    addr address;
};

class ABSXArgument : public Argument {
  public:
    ABSXArgument(addr address, const MachineSpec &machine) {
      this->address = machine.readAddr(address);
    }

    ostream &write(ostream& o) const {
      return o << "$" << hex << uppercase << setfill('0') << setw(4) << address << ",X";
    }

    addr getEncodedLength() const {
      return 2;
    }

  private:
    addr address;
};

class ABSYArgument : public Argument {
  public:
    ABSYArgument(addr address, const MachineSpec &machine) {
      this->address = machine.readAddr(address);
    }

    ostream &write(ostream& o) const {
      return o << "$" << hex << uppercase << setfill('0') << setw(4) << address << ",Y";
    }

    addr getEncodedLength() const {
      return 2;
    }

  private:
    addr address;
};

class IMMArgument : public Argument {
  public:
    IMMArgument(addr address, const MachineSpec &machine) {
      val = machine.readWord(address);
    }

    ostream &write(ostream& o) const {
      return o << "#$" << hex << uppercase << setw(2) << (int)val;
    }

    addr getEncodedLength() const {
      return 1;
    }

    virtual Value *getWordArgExpr(addr instructionAddr, BlockGenerator &blockgen) const {
      return blockgen.getConstant(val);
    }

  private:
    word val;
};

class INDYArgument : public Argument {
  public:
    INDYArgument(addr address, const MachineSpec &machine) {
      this->base = machine.readAddr(address);
    }

    ostream &write(ostream& o) const {
      return o << "($" << hex << uppercase << setfill('0') << setw(2) << (int)base << "),Y";
    }

    addr getEncodedLength() const {
      return 1;
    }

  private:
    word base;
};

class RELArgument : public Argument {
  public:
    RELArgument(addr address, const MachineSpec &machine) {
      offset = machine.readWord(address);
    }

    ostream &write(ostream& o) const {
      return o << "$" << hex << uppercase << setw(2) << (int)offset;
    }

    addr getEncodedLength() const {
      return 1;
    }

    virtual addr getAddrArg(addr instructionAddr) const {
      return instructionAddr + (*((int8_t *)&offset));
    }

  private:
    word offset;
};

class ZPGArgument : public Argument {
  public:
    ZPGArgument(addr address, const MachineSpec &machine) {
      this->address = machine.readWord(address);
    }

    ostream &write(ostream& o) const {
      return o << "$" << hex << uppercase << setw(2) << (int)address;
    }

    addr getEncodedLength() const {
      return 1;
    }

    virtual addr getAddrArg(addr instructionAddr) const {
      return address;
    }

    virtual bool isAbsolute() const {
      return true;
    }

  private:
    word address;
};

class NoArgInstruction : public Instruction {
  public:
    NoArgInstruction(addr location, const char *opcode) : Instruction(location, opcode) {}

    ostream &write(ostream& o) const {
      return o << hex << uppercase << setfill('0') << setw(4) << location << ": " << opcode;
    }

    addr getEncodedLength() const {
      return 1;
    }
};

class ArgInstruction : public Instruction {
  public:
    ArgInstruction(addr location, const char *opcode, Argument *arg) : 
      Instruction (location, opcode), 
      arg (arg) {}

    virtual ~ArgInstruction() {
      delete arg;
    }

    addr getEncodedLength() const {
      return 1 + arg->getEncodedLength();
    }

    ostream &write(ostream& o) const {
      return o << hex << uppercase << setfill('0') << setw(4) << location << ": " << opcode << " " << *arg;
    }

  protected:
    const Argument *arg;
};

Instruction::Instruction(addr location, const char *opcode) : 
  location(location),
  opcode (opcode)
{}

bool Instruction::isTerminal() const {
  return false;
}

bool Instruction::isBranch() const {
  return false;
}

addr Instruction::getBranchTarget() const {
  return 0;
}

addr Instruction::getFollowingLocation() const {
  return location + getEncodedLength();
}

void Instruction::generateCode(BlockGenerator &codegen) const { }

void setRegN(Value *val, BlockGenerator &blockgen) {
  Value *zero = blockgen.getConstant((word)0);
  Value *n = blockgen.getBuilder().CreateICmpSLT(val, zero);
  blockgen.setRegValue(REG_N, n);
}

void setRegZ(Value *val, BlockGenerator &blockgen) {
  Value *zero = blockgen.getConstant((word)0);
  Value *z = blockgen.getBuilder().CreateICmpEQ(val, zero);
  blockgen.setRegValue(REG_Z, z);
}

class LoadInstruction : public ArgInstruction {
  public:
    LoadInstruction(const char *opcode, Register reg, addr location, Argument *argument) :
    ArgInstruction(location, opcode, argument),
    reg(reg) { }

    virtual void generateCode(BlockGenerator &blockgen) const {
      Value *val = arg->getWordArgExpr(location, blockgen);
      blockgen.setRegValue(reg, val);
      setRegN(val, blockgen);
      setRegZ(val, blockgen);
    }

  private:
    Register reg;
};

class LDA : public LoadInstruction {
  public:
    LDA(addr location, Argument *argument) :
    LoadInstruction("LDA", REG_A, location, argument){ }
};

class LDX : public LoadInstruction {
  public:
    LDX(addr location, Argument *argument) :
    LoadInstruction("LDX", REG_X, location, argument){ }
};

class LDY : public LoadInstruction {
  public:
    LDY(addr location, Argument *argument) :
    LoadInstruction("LDY", REG_Y, location, argument){ }
};

class StoreInstruction : public ArgInstruction {
  public:
    StoreInstruction(const char *opcode, Register reg, addr location, Argument *argument) :
    ArgInstruction(location, opcode, argument),
    reg(reg) { }

    virtual void generateCode(BlockGenerator &blockgen) const {
      if (arg->isAbsolute()) {
        blockgen.getMachine().generateStore(arg->getAddrArg(location), blockgen.getRegValue(reg), blockgen);
      }
    }

  private:
    Register reg;
};

class STA : public StoreInstruction {
  public:
    STA(addr location, Argument *argument) :
    StoreInstruction("STA", REG_A, location, argument){ }
};

class STX : public StoreInstruction {
  public:
    STX(addr location, Argument *argument) :
    StoreInstruction("STX", REG_X, location, argument){ }
};

class STY : public StoreInstruction {
  public:
    STY(addr location, Argument *argument) :
    StoreInstruction("STY", REG_Y, location, argument){ }
};

class CompareInstruction : public ArgInstruction {
  public:
    CompareInstruction(const char *opcode, Register reg, addr location, Argument *argument) :
    ArgInstruction(location, opcode, argument),
    reg(reg) { }

    virtual void generateCode(BlockGenerator &blockgen) const {
      Value *argVal = arg->getWordArgExpr(location, blockgen);
      Value *regVal = blockgen.getRegValue(reg);
      Value *val = blockgen.getBuilder().CreateSub(regVal, argVal);
      setRegN(val, blockgen);
      setRegZ(val, blockgen);
    }

  private:
    Register reg;
};

class CMP : public CompareInstruction {
  public:
    CMP(addr location, Argument *argument) :
    CompareInstruction("CMP", REG_A, location, argument){ }
};

class CPX : public CompareInstruction {
  public:
    CPX(addr location, Argument *argument) :
    CompareInstruction("CPX", REG_X, location, argument){ }
};

class CPY : public CompareInstruction {
  public:
    CPY(addr location, Argument *argument) :
    CompareInstruction("CPY", REG_Y, location, argument){ }
};

class BranchInstruction : public ArgInstruction {
  public:
    BranchInstruction(const char *opcode, Register reg, bool inverse, addr location, Argument *argument) :
    ArgInstruction(location, opcode, argument),
    reg(reg),
    inverse(inverse){ }

    virtual bool isBranch() const {
      return true;
    }

    virtual addr getBranchTarget() const {
      return arg->getAddrArg(location + getEncodedLength());
    }

    virtual void generateCode(BlockGenerator &blockgen) const {
      Value *condition = blockgen.getRegValue(reg);
      addr trueBlock;
      addr falseBlock;
      if (inverse) {
        trueBlock = getFollowingLocation();
        falseBlock = getBranchTarget();
      } else {
        trueBlock = getBranchTarget();
        falseBlock = getFollowingLocation();
      }
      blockgen.generateConditionalJump(condition, trueBlock, falseBlock);
    }

  private:
    Register reg;
    bool inverse;
};

class BCS : public BranchInstruction {
  public:
    BCS(addr location, Argument *argument) :
    BranchInstruction("BCS", REG_C, false, location, argument){ }
};

class BNE : public BranchInstruction {
  public:
    BNE(addr location, Argument *argument) :
    BranchInstruction("BNE", REG_Z, true, location, argument){ }
};

class BPL : public BranchInstruction {
  public:
    BPL(addr location, Argument *argument) :
    BranchInstruction("BPL", REG_N, true, location, argument){ }
};

DEF_WORD_ARG_INST(BIT)
};

DEF_NO_ARG_INST(CLD)
};

DEF_NO_ARG_INST(DEX)
};

DEF_NO_ARG_INST(DEY)
};

DEF_ADDR_ARG_INST(INC)
};

DEF_NO_ARG_INST(INY)
};

DEF_ADDR_ARG_INST(JMP)
    virtual bool isTerminal() const {
      return true;
    }
};

DEF_ADDR_ARG_INST(JSR)
};

DEF_WORD_ARG_INST(ORA)
};

DEF_NO_ARG_INST(RTS)
    virtual bool isTerminal() const {
      return true;
    }
};

DEF_NO_ARG_INST(SEI)
};

DEF_NO_ARG_INST(TXS)
};

Instruction *readInstruction(addr address, const MachineSpec &machine) {
  word opcode = machine.readWord(address);

  switch(opcode) {
    case 0x09:
      return new ORA(address, new IMMArgument(address + 1, machine));
    case 0x10:
      return new BPL(address, new RELArgument(address + 1, machine));
    case 0x20:
      return new JSR(address, new ABSArgument(address + 1, machine));
    case 0x2C:
      return new BIT(address, new ABSArgument(address + 1, machine));
    case 0x4C:
      return new JMP(address, new ABSArgument(address + 1, machine));
    case 0x60:
      return new RTS(address);
    case 0x78:
      return new SEI(address);
    case 0x85:
      return new STA(address, new ZPGArgument(address + 1, machine));
    case 0x86:
      return new STX(address, new ZPGArgument(address + 1, machine));
    case 0x88:
      return new DEY(address);
    case 0x8D:
      return new STA(address, new ABSArgument(address + 1, machine));
    case 0x91:
      return new STA(address, new INDYArgument(address + 1, machine));
    case 0x99:
      return new STA(address, new ABSYArgument(address + 1, machine));
    case 0x9A:
      return new TXS(address);
    case 0xA0:
      return new LDY(address, new IMMArgument(address + 1, machine));
    case 0xA2:
      return new LDX(address, new IMMArgument(address + 1, machine));
    case 0xA9:
      return new LDA(address, new IMMArgument(address + 1, machine));
    case 0xAD:
      return new LDA(address, new ABSArgument(address + 1, machine));
    case 0xB0:
      return new BCS(address, new RELArgument(address + 1, machine));
    case 0xC0:
      return new CPY(address, new IMMArgument(address + 1, machine));
    case 0xC8:
      return new INY(address);
    case 0xC9:
      return new CMP(address, new IMMArgument(address + 1, machine));
    case 0xCA:
      return new DEX(address);
    case 0xD0:
      return new BNE(address, new RELArgument(address + 1, machine));
    case 0xBD:
      return new LDA(address, new ABSXArgument(address + 1, machine));
    case 0xD8:
      return new CLD(address);
    case 0xE0:
      return new CPX(address, new IMMArgument(address + 1, machine));
    case 0xEE:
      return new INC(address, new ABSArgument(address + 1, machine));
  }

  std::cerr << "Unknown instruction " << hex << setw(2) << setfill('0') << uppercase << (int)opcode << " at " << setw(4) << address << std::endl;
  //exit(1);
  return new RTS(address);

  return NULL;
}
