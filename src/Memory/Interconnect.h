#pragma once

#include "Simulation/MemorySystem.h"

namespace NVM::Memory {

class Interconnect : public NVM::Simulation::MemorySystem {
    public:
    bool issue(NVMainRequest* req);

    bool read(uint64_t address, NVM::Simulation::DataBlock data,
              unsigned int threadId, unsigned int cycle);
    bool write(uint64_t address, NVM::Simulation::DataBlock data,
               unsigned int threadId, unsigned int cycle);

    void cycle(unsigned int cycles);

    unsigned int getCurrentCycle();

    bool isEmpty() const;

    void printStats(std::ostream& statStream);
};

} // namespace NVM::Memory