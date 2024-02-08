#pragma once

#include "MemoryTypes/Component/BankState.h"

namespace NVM::ComponentType {

class OpenState : public BankState {
    public:
    OpenState(Connection<BankCommand>* cmd, Connection<BankResponse>* response,
              unsigned int row);

    void process();
    void cycle();

    std::unique_ptr<BankState> nextState();

    private:
    unsigned int row;
};

} // namespace NVM::ComponentType