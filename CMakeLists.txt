cmake_minimum_required(VERSION 3.0)

project(CeDImu)

set(SRCS
	src/CDI/CDI.cpp
	src/CDI/CDIDisk.cpp
	src/CDI/CDIDirectory.cpp
	src/CDI/CDIFile.cpp
	src/CDI/Export.cpp
	src/Cores/MCD212/DRAMInterface.cpp
	src/Cores/MCD212/MCD212.cpp
	src/Cores/MCD212/Display.cpp
	src/Cores/MCD212/Registers.cpp
	src/Cores/SCC66470/DRAMInterface.cpp
	src/Cores/SCC66470/SCC66470.cpp
	src/Cores/SCC68070/AddressingModes.cpp
	src/Cores/SCC68070/ConditionalTests.cpp
	src/Cores/SCC68070/Disassembler.cpp
	src/Cores/SCC68070/InstructionSet.cpp
	src/Cores/SCC68070/Interpreter.cpp
	src/Cores/SCC68070/MemoryAccess.cpp
	src/Cores/SCC68070/SCC68070.cpp
	src/GUI/DisassemblerFrame.cpp
	src/GUI/MainFrame.cpp
	src/GUI/GamePanel.cpp
	src/GUI/RAMSearchList.cpp
	src/GUI/RAMSearchFrame.cpp
	src/Config.cpp
	src/CeDImu.cpp
	src/main.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
set(CMAKE_SOURCE_DIRECTORY ".")

if(UNIX)
	set(CMAKE_GENERATOR "Unix Makefiles")
endif()

add_executable(CeDImu ${SRCS})

find_package(wxWidgets COMPONENTS adv core base REQUIRED)

if(wxWidgets_FOUND)
	include("${wxWidgets_USE_FILE}")
	target_compile_features(CeDImu PUBLIC cxx_std_11)
	target_include_directories(CeDImu PUBLIC ${wxWidgets_INCLUDE_DIRS})
	target_link_libraries(CeDImu ${wxWidgets_LIBRARIES})
	set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "-s -O3")
else()
	message(FATAL_ERROR "wxWidgets not found")
endif()
