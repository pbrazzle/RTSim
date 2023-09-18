/*******************************************************************************
 * Copyright (c) 2012-2014, The Microsystems Design Labratory (MDL)
 * Department of Computer Science and Engineering, The Pennsylvania State
 *University All rights reserved.
 *
 * This source code is part of NVMain - A cycle accurate timing, bit accurate
 * energy simulator for both volatile (e.g., DRAM) and non-volatile memory
 * (e.g., PCRAM). The source code is free and you can redistribute and/or
 * modify it by providing that the following conditions are met:
 *
 *  1) Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2) Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author list:
 *   Matt Poremba    ( Email: mrp5060 at psu dot edu
 *                     Website: http://www.cse.psu.edu/~poremba/ )
 *******************************************************************************/

#include "traceSim/traceMain.h"

#include "Decoders/DecoderFactory.h"
#include "Endurance/EnduranceDistributionFactory.h"
#include "include/NVMHelpers.h"
#include "Interconnect/InterconnectFactory.h"
#include "MemControl/MemoryControllerFactory.h"
#include "NVM/nvmain.h"
#include "SimInterface/NullInterface/NullInterface.h"
#include "src/AddressTranslator.h"
#include "src/Config.h"
#include "src/EventQueue.h"
#include "src/Interconnect.h"
#include "src/MemoryController.h"
#include "src/TranslationMethod.h"
#include "Utils/HookFactory.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <stdlib.h>

using namespace NVM;

class MainObj : public NVMObject {
    public:
    MainObj(Config* config) {
        outstandingRequests = 0;

        SetEventQueue(new EventQueue());
        SetGlobalEventQueue(new GlobalEventQueue());
        SetStats(new Stats());
        SetTagGenerator(new TagGenerator(1000));

        SimInterface* simInterface = new NullInterface();
        config->SetSimInterface(simInterface);
        simInterface->SetConfig(config, true);

        std::vector<std::string>& hookList = config->GetHooks();

        for (size_t i = 0; i < hookList.size(); i++) {
            std::cout << "Creating hook " << hookList[i] << std::endl;

            NVMObject* hook = HookFactory::CreateHook(hookList[i]);

            if (hook != NULL) {
                AddHook(hook);
                hook->SetParent(this);
                hook->Init(config);
            } else {
                std::cout << "Warning: Could not create a hook named `"
                          << hookList[i] << "'." << std::endl;
            }
        }

        NVMain* nvmain = new NVMain();
        AddChild(nvmain);
        nvmain->SetParent(this);

        globalEventQueue->SetFrequency(config->GetEnergy("CPUFreq") *
                                       1000000.0);
        globalEventQueue->AddSystem(nvmain, config);

        nvmain->SetConfig(config, "defaultMemory", true);

        std::cout << "traceMain (" << (void*) (this) << ")" << std::endl;
        nvmain->PrintHierarchy();
    }

    void Cycle(ncycle_t cycles) final { globalEventQueue->Cycle(cycles); }

    bool IsIssuable(NVMainRequest* req, FailReason* reason = nullptr) final {
        return GetChild()->IsIssuable(req, reason);
    }

    bool IssueCommand(NVMainRequest* req) final {
        auto requestCycle = req->arrivalCycle;
        std::cout << "TraceMain - Issuing request " << requestCycle
                  << " at cycle " << GetEventQueue()->GetCurrentCycle()
                  << std::endl;
        outstandingRequests++;
        return GetChild()->IssueCommand(req);
    }

    bool Drain() final {
        std::cout << "MainObj - Draining with " << outstandingRequests
                  << " remaining reqs\n";
        NVMObject::Drain();
        return outstandingRequests == 0;
    }

    bool RequestComplete(NVMainRequest* req) final {
        if (req->owner == this) {
            delete req;
            outstandingRequests--;
            return true;
        }
        throw std::runtime_error("Request with no owner!");
    }

    private:
    int outstandingRequests;
};

int main(int argc, char* argv[]) {
    TraceMain* traceRunner = new TraceMain();

    return traceRunner->RunTrace(argc, argv);
}

TraceMain::TraceMain() {
    ignoreData = false;
    trace = NULL;
}

TraceMain::~TraceMain() { delete config; }

void TraceMain::setupConfig(int argc, char* argv[]) {
    config = new Config();
    config->Read(argv[1]);

    // Allow for overriding config parameter values for trace simulations from
    // command line
    if (argc > 4) {
        for (int curArg = 4; curArg < argc; ++curArg) {
            std::string clParam, clValue, clPair;

            clPair = argv[curArg];
            clParam = clPair.substr(0, clPair.find_first_of("="));
            clValue =
                clPair.substr(clPair.find_first_of("=") + 1, std::string::npos);

            std::cout << "Overriding " << clParam << " with '" << clValue << "'"
                      << std::endl;

            config->SetValue(clParam, clValue);
        }
    }

    object = new MainObj(config);

    if (config->KeyExists("IgnoreData") &&
        config->GetString("IgnoreData") == "true") {
        ignoreData = true;
    }

    if (config->KeyExists("StatsFile")) {
        statStream.open(config->GetString("StatsFile").c_str(),
                        std::ofstream::out | std::ofstream::app);
    }

    if (config->KeyExists("TraceReader"))
        trace = TraceReaderFactory::CreateNewTraceReader(
            config->GetString("TraceReader"));
    else trace = TraceReaderFactory::CreateNewTraceReader("NVMainTrace");

    trace->SetTraceFile(argv[2]);

    if (argc == 3) simulateCycles = 0;
    else simulateCycles = atoi(argv[3]);
}

void printArgs(int argc, char* argv[]) {
    // Print out the command line that was provided
    std::cout << "NVMain command line is:" << std::endl;
    for (int curArg = 0; curArg < argc; ++curArg) {
        std::cout << argv[curArg] << " ";
    }
    std::cout << std::endl << std::endl;
}

NVMainRequest* TraceMain::getNextRequest() {
    TraceLine tl;
    if (!trace->GetNextAccess(&tl)) return nullptr;

    NVMainRequest* request = new NVMainRequest();
    request->address = tl.GetAddress();
    request->address2 = tl.GetAddress2();
    request->type = tl.GetOperation();
    request->bulkCmd = CMD_NOP;
    request->threadId = tl.GetThreadId();
    request->status = MEM_REQUEST_INCOMPLETE;
    if (!ignoreData) request->data = tl.GetData();
    if (!ignoreData) request->oldData = tl.GetOldData();
    request->owner = object;
    request->arrivalCycle = tl.GetCycle();

    return request;
}

void TraceMain::waitForDrain() {
    std::cout << "TraceMain - end of trace file reached\n";
    std::cout << "Could not read next line from trace file!" << std::endl;

    /* Wait for requests to drain. */
    while (!object->Drain()) {
        object->Cycle(1);
        currentCycle++;
    }
}

bool TraceMain::issueRequest(NVMainRequest* req) {
    while (!object->IsIssuable(req)) {
        if (!tryCycle(1)) return false;
    }

    auto requestCycle = req->arrivalCycle;
    std::cout << "TraceMain - Issuing request " << requestCycle << " at cycle "
              << currentCycle << std::endl;
    object->IssueCommand(req);

    return true;
}

bool TraceMain::tryCycle(ncycle_t cycles) {
    if (simulateCycles < (currentCycle + cycles) && simulateCycles > 0) {
        object->Cycle(simulateCycles - currentCycle);
        currentCycle += simulateCycles - currentCycle;
        return false;
    }

    object->Cycle(cycles);
    currentCycle += cycles;
    return true;
}

bool TraceMain::issueRowClone(NVMainRequest* req) {
    while (!object->IsIssuable(req)) {
        if (!tryCycle(1)) return false;
    }
    NVMainRequest* srcReq = req;
    srcReq->type = ROWCLONE_SRC;
    if (!object->IssueCommand(srcReq)) return false;

    NVMainRequest* destReq = new NVMainRequest();
    *destReq = *req;
    destReq->address = destReq->address2;
    destReq->type = ROWCLONE_DEST;
    if (!object->IssueCommand(destReq)) return false;
    return true;
}

void TraceMain::runSimulation() {
    std::cout << "*** Simulating " << simulateCycles << " input cycles. (";

    /*
     *  The trace cycle is assumed to be the rate that the CPU/LLC is issuing.
     *  Scale the simulation cycles to be the number of *memory cycles* to run.
     */
    simulateCycles = (uint64_t) ceil(((double) (config->GetValue("CPUFreq")) /
                                      (double) (config->GetValue("CLK"))) *
                                     simulateCycles);

    std::cout << simulateCycles << " memory cycles) ***" << std::endl;

    while (currentCycle <= simulateCycles || simulateCycles == 0) {
        std::cout << "TraceMain - Current cycle: " << currentCycle << std::endl;

        auto request = getNextRequest();

        if (!request) {
            waitForDrain();
            break;
        }

        auto requestCycle = request->arrivalCycle;

        if (config->KeyExists("IgnoreTraceCycle") &&
            config->GetString("IgnoreTraceCycle") == "true")
            requestCycle = 0;

        if (request->type != READ && request->type != WRITE &&
            request->type != PIM_OP)
            throw std::runtime_error("Unknown operation in trace file");

        if (requestCycle > currentCycle)
            if (!tryCycle(requestCycle - currentCycle)) break;

        if (request->type == PIM_OP) {
            if (!issueRowClone(request)) break;
        } else {
            if (!issueRequest(request)) break;
        }
    }
}

void TraceMain::printStats() {
    object->GetChild()->CalculateStats();
    std::ostream& refStream = (statStream.is_open()) ? statStream : std::cout;
    object->GetStats()->PrintAll(refStream);

    std::cout << "Exiting at cycle " << currentCycle << " because simCycles "
              << simulateCycles << " reached." << std::endl;
    if (outstandingRequests > 0)
        std::cout << "Note: " << outstandingRequests
                  << " requests still in-flight." << std::endl;
}

int TraceMain::RunTrace(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout
            << "Usage: nvmain CONFIG_FILE TRACE_FILE CYCLES [PARAM=value ...]"
            << std::endl;
        return 1;
    }

    setupConfig(argc, argv);
    printArgs(argc, argv);
    runSimulation();
    printStats();

    return 0;
}