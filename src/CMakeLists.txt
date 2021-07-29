cmake_minimum_required(VERSION 3.10)

project(CeDImu)

find_package(wxWidgets REQUIRED COMPONENTS aui adv core base)

if(NOT wxWidgets_FOUND)
    message(FATAL_ERROR "wxWidgets not found")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-s -O3")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(CDI)

add_executable(CeDImu
    CeDImu.cpp
    Config.cpp
    main.cpp
    GUI/CPUViewer.cpp
    GUI/DebugFrame.cpp
    GUI/GamePanel.cpp
    GUI/MainFrame.cpp
    GUI/RAMSearchFrame.cpp
    GUI/RAMSearchList.cpp
    GUI/SettingsFrame.cpp
    GUI/VDSCViewer.cpp
)

include(${wxWidgets_USE_FILE})
target_include_directories(CeDImu PUBLIC ${wxWidgets_INCLUDE_DIRS})
target_link_libraries(CeDImu cdi ${wxWidgets_LIBRARIES})

if(USE_STD_FILESYSTEM)
    target_link_libraries(CeDImu stdc++fs)
endif()
