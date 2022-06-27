cmake_minimum_required(VERSION 3.13)

project(CeDImu)

find_package(wxWidgets REQUIRED COMPONENTS aui adv core base)

if(NOT wxWidgets_FOUND)
    message(FATAL_ERROR "wxWidgets not found")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-s -O3 -DwxDEBUG_LEVEL=0")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(ENABLE_LOG ON)

add_subdirectory(CDI)

add_executable(CeDImu
    CeDImu.cpp
    Config.cpp
    main.cpp
    GUI/CPUViewer.cpp
    GUI/DebugFrame.cpp
    GUI/GamePanel.cpp
    GUI/MainFrame.cpp
    GUI/SettingsFrame.cpp
    GUI/VDSCViewer.cpp
    ../resources/resource.rc
)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipoAvailable)
if(ipoAvailable)
    set_property(TARGET cdi PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

include(${wxWidgets_USE_FILE})
target_include_directories(CeDImu PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_link_libraries(CeDImu cdi ${wxWidgets_LIBRARIES} stdc++fs)
