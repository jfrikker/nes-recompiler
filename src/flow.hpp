#pragma once

#include <set>

#include "llvm/IR/Module.h"

#include "memory.hpp"
#include "machine_spec.hpp"

class ModuleGenerator;

void identifyFunction(addr start, const MachineSpec &machine, std::set<addr> &out);
void identifyBlocks(addr start, const std::set<addr> &function, const MachineSpec &machine, std::set<addr> &out);
void findReachableFunctions(addr start, const MachineSpec &machine, std::set<addr> &out);
void writeFunction(addr start, bool external, ModuleGenerator &modgen);
