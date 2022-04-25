cmake_minimum_required(VERSION 3.10)

project(cdi)

find_package(wxWidgets REQUIRED)

if(NOT wxWidgets_FOUND)
    message(FATAL_ERROR "wxWidgets not found")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(NOT DEFINED LIBRARY_TYPE)
    set(LIBRARY_TYPE STATIC)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g -DENABLE_LOG")
set(CMAKE_CXX_FLAGS_RELEASE "-s -O3")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(ENABLE_LOG "Allows the library to print some messages in the console and the use of OnLogMemoryAccess callback" OFF)
if(ENABLE_LOG)
    add_definitions(-DENABLE_LOG)
endif()

add_library(cdi ${LIBRARY_TYPE}
    CDI.cpp
    CDIDisc.cpp
    CDIDirectory.cpp
    CDIFile.cpp
    Export.cpp
    PointingDevice.cpp
    boards/MiniMMC/Bus.cpp
    boards/MiniMMC/MiniMMC.cpp
    boards/Mono3/Bus.cpp
    boards/Mono3/Mono3.cpp
    common/Audio.cpp
    common/filesystem.cpp
    common/Video.cpp
    cores/DS1216/DS1216.cpp
    cores/M48T08/M48T08.cpp
    cores/MC68HC05/CoreTimer.cpp
    cores/MC68HC05/M68000Interface.cpp
    cores/MC68HC05/MC68HC05.cpp
    cores/MC68HC05/MC68HC05i8.cpp
    cores/MC68HC05/MC68HSC05C8.cpp
    cores/MC68HC05/ProgrammableTimer.cpp
    cores/MC68HC05/SCI.cpp
    cores/MCD212/Display.cpp
    cores/MCD212/DRAMInterface.cpp
    cores/MCD212/MCD212.cpp
    cores/MCD212/Registers.cpp
    cores/SCC66470/DRAMInterface.cpp
    cores/SCC66470/Registers.cpp
    cores/SCC66470/Display.cpp
    cores/SCC66470/SCC66470.cpp
    cores/SCC68070/AddressingModes.cpp
    cores/SCC68070/ConditionalTests.cpp
    cores/SCC68070/Disassembler.cpp
    cores/SCC68070/InstructionSet.cpp
    cores/SCC68070/Interpreter.cpp
    cores/SCC68070/MemoryAccess.cpp
    cores/SCC68070/Peripherals.cpp
    cores/SCC68070/SCC68070.cpp
    HLE/CIAP/CIAP.cpp
    HLE/IKAT/IKAT.cpp
    OS9/BIOS.cpp
    OS9/Stt.cpp
    OS9/SystemCalls.cpp
)

include(${wxWidgets_USE_FILE})
target_include_directories(cdi PRIVATE ${wxWidgets_INCLUDE_DIRS})
