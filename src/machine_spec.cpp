#include "machine_spec.hpp"

addr MachineSpec::readAddr(addr address) const {
  word little = readWord(address);
  word big = readWord(address + 1);
  return little + 256 * big;
}

addr MachineSpec::getNMIAddr() const {
  return readAddr(0xFFFA);
}

addr MachineSpec::getRSTAddr() const {
  return readAddr(0xFFFC);
}

addr MachineSpec::getBRKAddr() const {
  return readAddr(0xFFFE);
}
