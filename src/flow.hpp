#pragma once

#include <set>

#include "llvm/IR/Module.h"

#include "memory.hpp"

class ModuleGenerator;
class MachineSpec;

void identifyFunction(addr start, const MachineSpec &machine, std::set<addr> &out);
void identifyBlocks(addr start, const std::set<addr> &function, const MachineSpec &machine, std::set<addr> &out);
void writeFunction(addr start, ModuleGenerator &modgen);
