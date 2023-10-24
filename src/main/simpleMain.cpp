#include "Config.h"
#include "Logging/Logging.h"
#include "Memory/MemoryFactory.h"
#include "NVMTypes.h"
#include "Simulation/FileTraceReader.h"
#include "Simulation/TraceSimulator.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdlib.h>

using namespace NVM;
using namespace NVM::Memory;
using namespace NVM::Simulation;
using namespace NVM::Logging;

ncycle_t getMaxCycles(Config* config, char* arg) {
    ncycle_t maxCycles = 0;

    if (arg != nullptr) maxCycles = atoi(arg);

    return (uint64_t) ceil(((double) (config->GetValue("CPUFreq")) /
                            (double) (config->GetValue("CLK"))) *
                           maxCycles);
}

Config* createConfig(int argc, char* argv[]) {
    Config* config = new Config();
    config->Read(argv[1]);
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
    return config;
}

int main(int argc, char* argv[]) {
    // Check for correct args
    if (argc < 4) {
        std::cout
            << "Usage: nvmain CONFIG_FILE TRACE_FILE CYCLES [PARAM=value ...]"
            << std::endl;
        return 1;
    }

    // Print args
    std::cout << "NVMain command line is:" << std::endl;
    for (int curArg = 0; curArg < argc; ++curArg) {
        std::cout << argv[curArg] << " ";
    }
    std::cout << std::endl << std::endl;

    // Set up log
    Logging::setLogLevel(LogLevel::STAT);

    // Build dependencies
    Config* config = createConfig(argc, argv);
    ncycle_t simulateCycles =
        getMaxCycles(config, (argc > 3) ? argv[3] : nullptr);

    // Build RTSystem
    auto memory = makeSimpleSystem();

    // Build TraceReader
    auto reader = std::make_unique<FileTraceReader>(argv[2]);

    // Build simulator and run
    TraceSimulator simulator(std::move(reader), std::move(memory),
                             simulateCycles);
    simulator.run();
    simulator.printStats(std::cout);

    return 0;
}