#pragma once

#include <set>

#include "memory.hpp"

class ModuleGenerator;
class MachineSpec;

void identifyFunction(addr start, const MachineSpec &machine, std::set<addr> &out);
void identifyBlocks(addr start, const std::set<addr> &function, const MachineSpec &machine, std::set<addr> &out);
void findReachableFunctions(addr start, const MachineSpec &machine, std::set<addr> &out);
void writeFunction(addr start, bool external, ModuleGenerator &modgen);
