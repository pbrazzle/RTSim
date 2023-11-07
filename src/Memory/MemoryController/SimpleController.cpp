#include "Memory/MemoryController/SimpleController.h"

#include "Logging/Logging.h"
#include "Memory/Command/WaitingCommand.h"

#include <functional>

using namespace NVM::Memory;

using namespace NVM::Memory;
using namespace NVM::Simulation;
using namespace NVM::Logging;

using CommandFunc = std::function<Command*()>;

std::unique_ptr<Command> makeControllerCommand(CommandFunc& func) {
    auto interconnectCommand = func();
    if (!interconnectCommand) return nullptr;

    auto systemCommand = std::unique_ptr<Command>(new WaitingCommand());
    interconnectCommand->setParent(systemCommand.get());
    return std::move(systemCommand);
}

SimpleController::SimpleController() :
    totalReads(0),
    totalWrites(0),
    bankQueues(1) {}

bool SimpleController::read(uint64_t address, DataBlock data) {
    if (interconnects.empty()) return false;
    if (systemCmd) return false;
    if (bankQueues[0].size() == 10) return false;

    CommandFunc readFunc = [this, address, data]() {
        ReadInstruction inst(address, data);
        return interconnects[0]->issue(inst);
    };

    systemCmd.reset(new TriggeredCommand(readFunc));
    log() << LogLevel::EVENT << "SimpleController received read\n";
    totalReads++;

    return true;
}

bool SimpleController::write(uint64_t address,
                                 NVM::Simulation::DataBlock data) {
    if (interconnects.empty()) return false;
    if (systemCmd) return false;
    if (bankQueues[0].size() == 10) return false;

    CommandFunc writeFunc = [this, address, data]() {
        WriteInstruction inst(address, data);
        return interconnects[0]->issue(inst);
    };

    systemCmd.reset(new TriggeredCommand(writeFunc));
    log() << LogLevel::EVENT << "SimpleController received write\n";
    totalWrites++;

    return true;
}

bool SimpleController::rowClone(uint64_t srcAddress, uint64_t destAddress,
                                    NVM::Simulation::DataBlock data) {
    return false;
}

bool
SimpleController::transverseRead(uint64_t baseAddress, uint64_t destAddress,
                                 std::vector<NVM::Simulation::DataBlock> data) {
    return false;
}

bool SimpleController::transverseWrite(
    uint64_t address, std::vector<NVM::Simulation::DataBlock> data) {
    return false;
}

void SimpleController::cycle(unsigned int cycles) {
    if (cycles == 0) return;
    if (!interconnects.empty()) interconnects[0]->cycle(cycles);

    if (systemCmd) {
        log() << LogLevel::DEBUG
              << "SimpleController: Moving issued command to bank queue\n";
        bankQueues[0].push(std::move(systemCmd));
        systemCmd.reset();
    }
    while (cycles && !bankQueues[0].empty()) {
        bankQueues[0].front()->cycle(1);
        if (bankQueues[0].front()->isDone()) {
            log() << LogLevel::DEBUG
                  << "SimpleController: Queue command finished\n";
            bankQueues[0].pop();
        }
        issueFromQueue();

        cycles--;
    }
}

void SimpleController::issueFromQueue() {
    if (!bankQueues[0].empty() && !bankQueues[0].front()->triggered()) {
        log() << LogLevel::DEBUG
              << "SimpleController: Issued command from queue\n";
        bankQueues[0].front()->issue();
    }
}

bool SimpleController::isEmpty() const { 
    if (interconnects.empty()) return false;
    return interconnects[0]->isEmpty() && bankQueues[0].empty() && !systemCmd;
}

StatBlock SimpleController::getStats(std::string tag) const {
    StatBlock stats(tag);

    stats.addStat(&totalReads, "reads");
    stats.addStat(&totalWrites, "writes");

    for (int i = 0; i < interconnects.size(); i++) {
        stats.addChild(
            interconnects[i]->getStats(tag + ".i" + std::to_string(i)));
    }

    return stats;
}