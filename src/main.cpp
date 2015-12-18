#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory>
#include <unistd.h>
#include <iostream>
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

void *mmapWholeFile(const char *filename, int &fd, off_t &len) {
  fd = open(filename, O_RDONLY);

  struct stat s;
  fstat(fd, &s);
  len = s.st_size;

  return mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
}

int main(int argc, char **argv) {
  boost::iostreams::mapped_file file(argv[1]);

  NesMachineSpec *machine;
  if(!(machine = loadNesMachine((const word *)file.data()))) {
    fprintf(stderr, "Not a valid header file\n");
    return 1;
  }

  //std::cout.write((const char *)machine->getPrgRom(), machine->getPrgRomSize());
  //std::cout << std::hex << machine->getPrgRomOffset() << std::endl;
  
  addr address = 0x90CC; //machine->getRSTAddr();
  //addr address = machine->getRSTAddr();
  // std::cout << std::hex << machine->getPrgRomSize() << std::endl;
  // std::cout << std::hex << address << std::endl;

  // std::cout << std::hex << (int)machine->readWord(address) << std::endl;
  std::set<addr> function;
  std::set<addr> blocks;
  identifyFunction(address, *machine, function);
  identifyBlocks(address, function, *machine, blocks);
  for (std::set<addr>::iterator it = function.begin(); it != function.end(); it++) {
    if (blocks.count(*it)) {
      std::cout << "-- ";
    } else {
      std::cout << "   ";
    }

    std::unique_ptr<Instruction> inst(readInstruction(*it, *machine));
    std::cout << *inst << std::endl;
  }

  ModuleGenerator modgen("mymod", *machine);
  machine->writeLLVMHeader(modgen);
  writeFunction(address, modgen);

  modgen.write();

  delete machine;
  return 0;
}
