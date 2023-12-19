#include "Logging/Logging.h"
#include "Memory/MemoryFactory.h"
#include "Modeling/Decoder.h"
#include "Simulation/Config.h"
#include "Simulation/TraceReader/FileTraceReader.h"
#include "Simulation/TraceSimulator.h"
#include "src/old/NVMTypes.h"

#include <iostream>
#include <memory>
#include <stdlib.h>

using namespace NVM;
using namespace NVM::Memory;
using namespace NVM::Simulation;
using namespace NVM::Logging;
using namespace NVM::Modeling;

ncycle_t getMaxCycles(char* arg) {
    if (arg != nullptr) return atoi(arg);
    return 0;
}

void setAddressScheme(const NVM::Simulation::Config& conf) {
    ComponentCounts counts;

    counts.rows = conf.get<int>("DBCS");
    counts.cols = conf.get<int>("DOMAINS");
    counts.ranks = conf.get<int>("RANKS");
    counts.banks = conf.get<int>("BANKS");
    counts.channels = conf.get<int>("CHANNELS");

    setScheme(conf.get<std::string>("AddressMappingScheme"), counts);
}

void setLogLevel(const NVM::Simulation::Config& conf) {
    try {
        auto logLevel = conf.get<std::string>("LogLevel");

        if (logLevel == "DEBUG") {
            log().setGlobalLevel(LogLevel::DEBUG);
        } else if (logLevel == "EVENT") {
            log().setGlobalLevel(LogLevel::EVENT);
        } else if (logLevel == "STAT") {
            log().setGlobalLevel(LogLevel::STAT);
        } else if (logLevel == "ERROR") {
            log().setGlobalLevel(LogLevel::ERROR);
        }
    } catch (...) {
        log().setGlobalLevel(LogLevel::STAT);
    }
}

void setSeed() { srand(time(nullptr)); }

int main(int argc, char* argv[]) {
    // Check for correct args
    if (argc < 4) {
        std::cout
            << "Usage: nvmain CONFIG_FILE TRACE_FILE CYCLES [PARAM=value ...]"
            << std::endl;
        return 1;
    }

    setSeed();

    // Build dependencies
    ncycle_t simulateCycles = getMaxCycles((argc > 3) ? argv[3] : nullptr);

    NVM::Simulation::Config conf = readConfig(argv[1]);
    setAddressScheme(conf);
    setLogLevel(conf);

    // Build RTSystem
    auto memory = makeSimpleSystem(conf);

    // Build TraceReader
    auto reader = std::make_unique<FileTraceReader>(argv[2]);

    // Build simulator and run
    TraceSimulator simulator(std::move(reader), std::move(memory),
                             simulateCycles);
    simulator.run();
    simulator.printStats(std::cout);

    return 0;
}