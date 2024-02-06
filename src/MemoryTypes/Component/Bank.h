#pragma once

#include "MemoryTypes/Component/BankResponse.h"
#include "MemoryTypes/Component/Component.h"
#include "MemoryTypes/Component/Connection.h"
#include "MemoryTypes/Component/MemoryCommand.h"

namespace NVM::ComponentType {

class Bank : public Component {
    public:
    Bank();

    /*
     *  Processes incoming commands from upstream Components
     */
    void process();

    /*
     *  Sends outgoing commands to downstream Components
     */
    void cycle();

    /*
     *  Returns all stat values associated with this Component
     */
    Stats::StatBlock getStats();

    private:
    Connection<BankResponse>* responseConnection;
    Connection<MemoryCommand>* commandConnection;
};

} // namespace NVM::ComponentType