cmake_minimum_required(VERSION 3.13)

find_package(wxWidgets REQUIRED COMPONENTS base)

if(NOT wxWidgets_FOUND)
    message(FATAL_ERROR "wxWidgets not found")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DwxDEBUG_LEVEL=0")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(ENABLE_LOG "Allows the library to print some messages in the console and the use of OnLogMemoryAccess callback" OFF)
if(ENABLE_LOG)
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

include(${wxWidgets_USE_FILE})
target_include_directories(CeDImu PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_link_libraries(CeDImu ${wxWidgets_LIBRARIES} stdc++fs)
