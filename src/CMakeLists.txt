cmake_minimum_required(VERSION 3.13)

project(CeDImu-GUI)

find_package(wxWidgets REQUIRED COMPONENTS aui adv core base)

if(NOT wxWidgets_FOUND)
    message(FATAL_ERROR "wxWidgets not found")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-s -O3 -DwxDEBUG_LEVEL=0")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(ENABLE_LOG ON)

add_subdirectory(CDI)

add_executable(CeDImu-GUI
    CeDImu.cpp
    CeDImu.hpp
    Config.cpp
    Config.hpp
    main.cpp
    ../resources/resource.rc
)
add_subdirectory(GUI)
set_target_properties(CeDImu-GUI PROPERTIES OUTPUT_NAME "CeDImu")

include(CheckIPOSupported)
check_ipo_supported(RESULT ipoAvailable)
if(ipoAvailable)
    set_property(TARGET CeDImu-GUI PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

include(${wxWidgets_USE_FILE})
target_include_directories(CeDImu-GUI PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_link_libraries(CeDImu-GUI CeDImu ${wxWidgets_LIBRARIES} stdc++fs)
