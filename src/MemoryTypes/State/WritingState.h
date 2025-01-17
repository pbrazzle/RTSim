#pragma once

#include "MemoryTypes/State/State.h"

namespace NVM::State {
class WritingState : public State {
    public:
    WritingState();

    /*
     *   Progresses the State forward by 1 cycle.
     *
     *   @return Next State
     */
    void cycle();

    std::unique_ptr<State> getNext() const;

    /*
     *   Performs a READ command.
     */
    bool read(const Address& address, const RowData& data);

    /*
     *   Performs a WRITE command.
     */
    bool write(const Address& address, const RowData& data);

    /*
     *   Performs an ACTIVATE command.
     */
    bool activate(const Address& address);

    /*
     *   Performs a PRECHARGE command.
     */
    bool precharge(const Address& address);

    /*
     *   Performs a REFRESH command.
     */
    bool refresh();

    bool finished() const;

    Stats::ValueStatBlock getStats() const;

    private:
    int remainingCycles;
};
} // namespace NVM::State