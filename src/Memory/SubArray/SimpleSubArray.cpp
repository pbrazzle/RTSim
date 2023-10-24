#include "Memory/SubArray/SimpleSubArray.h"

#include "Logging/Logging.h"

namespace NVM::Memory {

class TimedCommand : public Command {
    public:
    TimedCommand() : parent(nullptr), cyclesLeft(5) {}

    void setParent(Command* p) { parent = p; }

    void cycle(unsigned int cycles) {
        if (cycles >= cyclesLeft) {
            cyclesLeft = 0;
            if (parent) parent->notify();
            return;
        }
        cyclesLeft -= cycles;
    }

    bool isDone() { return cyclesLeft == 0; }

    void notify() {}

    private:
    Command* parent;
    unsigned int cyclesLeft;
};

} // namespace NVM::Memory

using namespace NVM::Memory;
using namespace NVM::Logging;

SimpleSubArray::SimpleSubArray() :
    totalReads(0),
    totalWrites(0),
    currentCommand(nullptr) {}

Command* SimpleSubArray::read(uint64_t address,
                              NVM::Simulation::DataBlock data) {
    if (currentCommand) return nullptr;
    currentCommand = std::unique_ptr<Command>(new TimedCommand());
    if (currentCommand) {
        log() << LogLevel::EVENT << "SubArray received read\n";
        totalReads++;
    }
    return currentCommand.get();
}

Command* SimpleSubArray::write(uint64_t address,
                               NVM::Simulation::DataBlock data) {
    if (currentCommand) return nullptr;
    currentCommand = std::unique_ptr<Command>(new TimedCommand());
    if (currentCommand) {
        log() << LogLevel::EVENT << "SubArray received write\n";
        totalWrites++;
    }
    return currentCommand.get();
}

void SimpleSubArray::cycle(unsigned int cycles) {
    if (!currentCommand) return;
    TimedCommand* cmd = static_cast<TimedCommand*>(currentCommand.get());
    cmd->cycle(cycles);
    if (cmd->isDone()) currentCommand.reset();
}

bool SimpleSubArray::isEmpty() const { return currentCommand == nullptr; }

StatBlock SimpleSubArray::getStats(std::string tag) const {
    StatBlock stats(tag);

    stats.addStat(&totalReads, "reads");
    stats.addStat(&totalWrites, "writes");

    return stats;
}