cmake_minimum_required(VERSION 3.13)

project(libCeDImu)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(CEDIMU_ENABLE_LOG "Allows the library to print some messages in the console and the use of OnLogMemoryAccess callback" OFF)
if(CEDIMU_ENABLE_LOG)
    add_definitions(-DENABLE_LOG)
endif()

add_library(CeDImu ${LIBRARY_TYPE}
    CDI.cpp
    CDI.hpp
    CDIConfig.hpp
    CDIDirectory.cpp
    CDIDirectory.hpp
    CDIDisc.cpp
    CDIDisc.hpp
    CDIFile.cpp
    CDIFile.hpp
    Export.cpp
    PointingDevice.cpp
    PointingDevice.hpp
)

add_subdirectory(boards)
add_subdirectory(common)
add_subdirectory(cores)
add_subdirectory(HLE)
add_subdirectory(OS9)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(CeDImu PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(CeDImu PRIVATE stdc++fs)
endif()
