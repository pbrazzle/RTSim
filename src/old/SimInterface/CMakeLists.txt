set(INTERFACE_SOURCES
    #Gem5Interface/Gem5Interface.cpp
    ${RTSIM_DIR}/src/old/SimInterface/NullInterface/NullInterface.cpp)

set(ALL_SOURCES ${ALL_SOURCES} ${INTERFACE_SOURCES} PARENT_SCOPE)