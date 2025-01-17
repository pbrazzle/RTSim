#pragma once

#include "MemoryTypes/Component/Bank/BankCommand.h"
#include "MemoryTypes/Component/Controller/BankModel.h"
#include "MemoryTypes/Component/System/MemoryCommand.h"

#include <vector>

namespace NVM::ComponentType {

class Parser {
    public:
    Parser() = default;

    std::vector<BankCommand> parse(MemoryCommand cmd, BankModel& bank);
};

} // namespace NVM::ComponentType