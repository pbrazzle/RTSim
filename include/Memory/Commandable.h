#pragma once

#include "Utils/Address.h"
#include "Utils/RowData.h"

#include <functional>
#include <iostream>
#include <vector>

namespace NVM::Memory {

using NVM::Address;
using NVM::RowData;

class Commandable {
    public:
    virtual bool read(const Address& address, const RowData& data) = 0;

    virtual bool write(const Address& address, const RowData& data) = 0;

    virtual bool rowClone(const Address& srcAddress, const Address& destAddress,
                          const RowData& data) = 0;

    virtual bool refresh(const Address& bankAddress) = 0;

    virtual bool pim(std::vector<Address> operands, const Address& destAddress,
                     std::vector<RowData> data) = 0;

    virtual bool isEmpty() const = 0;

    virtual void cycle(unsigned int cycles = 1) = 0;

    virtual void printStats(std::ostream& statStream) = 0;

    virtual ~Commandable() {}
};

using Command = std::function<bool(Commandable&)>;

} // namespace NVM::Simulation