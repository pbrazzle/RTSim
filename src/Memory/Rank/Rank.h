#pragma once

#include "Memory/Bank/Bank.h"
#include "Simulation/MemorySystem.h"

#include <memory>

namespace NVM::Memory {

class Rank {
    public:
    Command* issue(Instruction& instruction);

    virtual bool isEmpty() const = 0;

    virtual void cycle(unsigned int cycles) = 0;

    virtual StatBlock getStats(std::string tag) const = 0;

    void addBank(std::unique_ptr<Bank> bank);

    protected:
    std::vector<std::unique_ptr<Bank>> banks;
};

} // namespace NVM::Memory