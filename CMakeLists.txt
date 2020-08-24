cmake_minimum_required(VERSION 3.0)

project(CeDImu)

set(SRCS
	src/CDI/CDI.cpp
	src/CDI/CDIDisk.cpp
	src/CDI/CDIDirectory.cpp
	src/CDI/CDIFile.cpp
	src/CDI/Export.cpp
	src/Boards/Mono3.cpp
	src/common/Audio.cpp
	src/common/Video.cpp
	src/cores/MCD212/DRAMInterface.cpp
	src/cores/MCD212/MCD212.cpp
	src/cores/MCD212/Display.cpp
	src/cores/MCD212/Registers.cpp
	src/cores/SCC66470/DRAMInterface.cpp
	src/cores/SCC66470/SCC66470.cpp
	src/cores/SCC68070/AddressingModes.cpp
	src/cores/SCC68070/ConditionalTests.cpp
	src/cores/SCC68070/Disassembler.cpp
	src/cores/SCC68070/InstructionSet.cpp
	src/cores/SCC68070/Interpreter.cpp
	src/cores/SCC68070/MemoryAccess.cpp
	src/cores/SCC68070/Peripherals.cpp
	src/cores/SCC68070/SCC68070.cpp
	src/GUI/DisassemblerFrame.cpp
	src/GUI/MainFrame.cpp
	src/GUI/GamePanel.cpp
	src/GUI/RAMSearchList.cpp
	src/GUI/RAMSearchFrame.cpp
	src/GUI/VDSCViewer.cpp
	src/Config.cpp
	src/CeDImu.cpp
	src/main.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
set(CMAKE_SOURCE_DIRECTORY ".")

# comment to use wxWidgets file system functions.
# uncomment to use C++17 file system header
# add_definitions(-DUSE_STD_FILESYSTEM)

if(UNIX)
	set(CMAKE_GENERATOR "Unix Makefiles")
endif()

find_package(wxWidgets REQUIRED COMPONENTS adv core base)

if(wxWidgets_FOUND)
	add_executable(CeDImu ${SRCS})
	include("${wxWidgets_USE_FILE}")
	target_compile_features(CeDImu PUBLIC cxx_std_17)
	target_include_directories(CeDImu PUBLIC ${wxWidgets_INCLUDE_DIRS})
	target_link_libraries(CeDImu ${wxWidgets_LIBRARIES})
	set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "-s -O3")
else()
	message(FATAL_ERROR "wxWidgets not found")
endif()
