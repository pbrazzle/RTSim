#include "Memory/SubArray/RowController.h"

#include "Logging/Logging.h"
#include "Memory/Command/NullCommand.h"
#include "Memory/Command/TimedCommand.h"

using namespace NVM::Memory;
using namespace NVM::Logging;

RowController::RowController(unsigned int rows, RowTiming timing) :
    currentCmd(nullptr),
    rowStates(rows, RowState::CLOSED),
    prechargeTime(timing.prechargeTime),
    activateTime(timing.activateTime),
    openRow(-1) {}

RowState RowController::getState(unsigned int row) const {
    return rowStates[row];
}

void RowController::setState(unsigned int row, RowState state) {
    rowStates[row] = state;
}

RowState& RowController::operator[](unsigned int row) { return rowStates[row]; }

const RowState& RowController::operator[](unsigned int row) const {
    return rowStates.at(row);
}

Command* RowController::activate(unsigned int row) {
    if (currentCmd.get()) return nullptr;
    if (openRow != -1) return nullptr;
    if (openRow == row) return sendNull();
    currentCmd.reset(new TimedCommand(activateTime));
    log() << LogLevel::EVENT << "SubArray received activate for row " << row
          << '\n';
    openRow = row;
    rowStates[row] = RowState::OPEN;
    return currentCmd.get();
}

Command* RowController::precharge(unsigned int row) {
    if (currentCmd.get()) return nullptr;
    if (openRow != row) return nullptr;
    if (openRow == -1) return sendNull();
    currentCmd.reset(new TimedCommand(prechargeTime));
    log() << LogLevel::EVENT << "SubArray received precharge for row " << row
          << '\n';
    openRow = -1;
    rowStates[row] = RowState::CLOSED;
    return currentCmd.get();
}

void RowController::cycle(unsigned int cycles) {
    if (currentCmd) {
        currentCmd->cycle(cycles);
        if (currentCmd->isDone()) currentCmd.reset();
    }
}

Command* RowController::closeRow() {
    if (openRow != -1) return precharge(openRow);
    return sendNull();
}

bool RowController::rowIsOpen(unsigned int row) {
    return rowStates[row] == RowState::OPEN;
}