set(TEST_SOURCES
    MemoryFactory.test.cpp)

add_catch_tests(TARGETS ${TEST_SOURCES} LIBS RTSim_Memory RTSim_Logging RTSim_Simulation)
