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
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-s -O3")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(USE_STD_FILESYSTEM "Use the C++17 filesystem functions" OFF)
option(FILESYSTEM_EXPERIMENTAL "Include <experimental/filesystem> instead of <filesystem>" OFF)

if(USE_STD_FILESYSTEM)
    add_definitions(-DUSE_STD_FILESYSTEM)
    if(FILESYSTEM_EXPERIMENTAL)
        add_definitions(-DFILESYSTEM_EXPERIMENTAL)
    endif()
endif()

add_library(cdi ${LIBRARY_TYPE}
    CDI.cpp
    CDIDisc.cpp
    CDIDirectory.cpp
    CDIFile.cpp
    Export.cpp
    boards/MiniMMC/Bus.cpp
    boards/MiniMMC/MiniMMC.cpp
    boards/Mono3/Bus.cpp
    boards/Mono3/Mono3.cpp
    common/Audio.cpp
    common/filesystem.cpp
    common/Video.cpp
    cores/M48T08/M48T08.cpp
    cores/MC68HC705C8/Interpreter.cpp
    cores/MC68HC705C8/MC68HC705C8.cpp
    cores/MC68HC705C8/MemoryAccess.cpp
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
    HLE/IKAT/IKAT.cpp
    OS9/BIOS.cpp
)

include(${wxWidgets_USE_FILE})
target_include_directories(cdi PUBLIC ${wxWidgets_INCLUDE_DIRS})
