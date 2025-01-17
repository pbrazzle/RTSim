#pragma once

#include "src/old/BankCounter.h"
#include "src/old/EventQueue.h"
#include "src/old/NVMTypes.h"
#include "src/old/Params.h"

namespace NVM {
class MemoryController;

class RefreshHandler {
    public:
    RefreshHandler();

    RefreshHandler(MemoryController* parent, Params* params,
                   EventQueue* parentQueue);

    bool NeedRefresh(const ncounter_t, const ncounter_t);

    /*
     * Returns true if ALL command queues in the bank group are empty
     * it simply checks all banks in the refresh bank group whether their
     * command queues are empty. the result is the union of each check
     */
    bool IsRefreshBankQueueEmpty(const ncounter_t, const ncounter_t);

    bool needsRefresh(NVMAddress address);

    bool canRefresh(NVMAddress address);

    // True if any bank group can refresh
    bool canRefresh();

    /*
     * Sets the refresh flag for a given bank group
     */
    void SetRefresh(const ncounter_t, const ncounter_t);

    /*
     * Resets the refresh flag for a given bank group
     */
    void ResetRefresh(const ncounter_t, const ncounter_t);

    /*
     * Resets the refresh queued flag for a given bank group
     */
    void ResetRefreshQueued(const ncounter_t bank, const ncounter_t rank);

    /*
     * Increments the delayedRefreshCounter in a given bank group
     */
    void IncrementRefreshCounter(const ncounter_t, const ncounter_t);

    /*
     * Decrements the delayedRefreshCounter in a given bank group
     */
    void DecrementRefreshCounter(const ncounter_t, const ncounter_t);

    /*
     * Issues REFRESH command if necessary
     */
    bool HandleRefresh();

    /*
     * Increments the delayedRefreshCounter and generate the next
     * refresh pulse
     */
    void ProcessRefreshPulse(NVMainRequest*);

    bool refreshQueued(NVMainRequest*);

    bool bankNeedsRefresh(NVMainRequest*);

    ncycle_t getLastRefresh();

    private:
    MemoryController* parent;
    Params* params;
    EventQueue* parentQueue;

    BankCounter delayedRefreshCounter, needsRefreshCounter, queued;
    ncounter_t m_refreshBankNum;
    ncycle_t m_tREFI, lastRefresh;
    ncounter_t nextRefreshRank, nextRefreshBank;
};
} // namespace NVM