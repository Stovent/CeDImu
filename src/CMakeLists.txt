cmake_minimum_required(VERSION 3.0)

project(CeDImu)

find_package(wxWidgets REQUIRED COMPONENTS aui adv core base)

if(NOT wxWidgets_FOUND)
	message(FATAL_ERROR "wxWidgets not found")
endif()

add_subdirectory(CDI)

add_executable(CeDImu
    CeDImu.cpp
    Config.cpp
    main.cpp
    GUI/CPUViewer.cpp
    GUI/GamePanel.cpp
    GUI/GenericList.cpp
    GUI/MainFrame.cpp
    GUI/RAMSearchFrame.cpp
    GUI/RAMSearchList.cpp
    GUI/SlaveViewer.cpp
    GUI/VDSCViewer.cpp
)

include(${wxWidgets_USE_FILE})
target_include_directories(CeDImu PUBLIC ${wxWidgets_INCLUDE_DIRS})
target_link_libraries(CeDImu cdi ${wxWidgets_LIBRARIES})
