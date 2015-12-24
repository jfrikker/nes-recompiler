#include <cstdint>
#include <cstring>
#include <memory>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <set>

#include <boost/iostreams/device/mapped_file.hpp>
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"

#include "memory.hpp"
#include "nes_machine_spec.hpp"
#include "instruction.hpp"
#include "flow.hpp"
#include "codegen.hpp"

int main(int argc, char **argv) {
  boost::iostreams::mapped_file file(argv[1]);

  NesMachineSpec *machine;
  if(!(machine = loadNesMachine((const word *)file.data()))) {
    fprintf(stderr, "Not a valid header file\n");
    return 1;
  }

  //std::cout.write((const char *)machine->getPrgRom(), machine->getPrgRomSize());
  //std::cout << std::hex << machine->getPrgRomOffset() << std::endl;
  
  addr address = machine->getRSTAddr();
  //addr address = 0x8E19;
  // std::cout << std::hex << machine->getPrgRomSize() << std::endl;
  // std::cout << std::hex << address << std::endl;

  // std::cout << std::hex << (int)machine->readWord(address) << std::endl;

  std::set<addr> functions;
  findReachableFunctions(address, *machine, functions);

  ModuleGenerator modgen("mymod", *machine);
  machine->writeLLVMHeader(modgen);

  for (auto funcStart : functions) {
    declareFunction(funcStart, (funcStart == address), modgen);
  }

  for (auto funcStart : functions) {
    std::set<addr> function;
    std::set<addr> blocks;
    identifyFunction(funcStart, *machine, function);
    identifyBlocks(funcStart, function, *machine, blocks);
    for (std::set<addr>::iterator it = function.begin(); it != function.end(); it++) {
      if (blocks.count(*it)) {
        std::cout << "-- ";
      } else {
        std::cout << "   ";
      }

      std::unique_ptr<Instruction> inst(readInstruction(*it, *machine));
      std::cout << *inst << std::endl;
    }
    std::cout << std::endl;

    writeFunction(funcStart, modgen);
  }

  modgen.write();

  delete machine;
  return 0;
}
