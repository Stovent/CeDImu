cmake_minimum_required(VERSION 3.15)

find_package(wxWidgets REQUIRED COMPONENTS aui adv core base)

if(NOT wxWidgets_FOUND)
    message(FATAL_ERROR "wxWidgets not found")
endif()

option(CEDIMU_ENABLE_LTO "Enable LTO if available" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(ENABLE_LOG ON)

add_subdirectory(CDI)

add_executable(CeDImu-GUI WIN32
    CeDImu.cpp
    CeDImu.hpp
    Config.cpp
    Config.hpp
    main.cpp
)
add_subdirectory(GUI)

set_target_properties(CeDImu-GUI PROPERTIES OUTPUT_NAME "CeDImu")

if(CMAKE_BUILD_TYPE STREQUAL "Release") # TODO: check if CMAKE_CONFIGURATION_TYPES also contains "Release".
    target_compile_definitions(CeDImu-GUI PRIVATE wxDEBUG_LEVEL=0)
endif()

if(CEDIMU_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipoAvailable)
    if(ipoAvailable)
        set_property(TARGET CeDImu-GUI PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

include(${wxWidgets_USE_FILE})
target_include_directories(CeDImu-GUI PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_link_libraries(CeDImu-GUI CeDImu ${wxWidgets_LIBRARIES})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(CeDImu-GUI PRIVATE -Wall -Wextra)
    target_link_options(CeDImu-GUI PRIVATE -static-libgcc -static-libstdc++)
    target_link_libraries(CeDImu-GUI stdc++fs)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_property(TARGET CeDImu-GUI PROPERTY MSVC_RUNTIME_LIBRARY ${CMAKE_MSVC_RUNTIME_LIBRARY}) # To set on the CLI.
endif()

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Bug of wxWidgets 3.2.0 on LLVM's Clang.
    target_sources(CeDImu-GUI
        PRIVATE
            ../resources/resource.rc
    )
endif()
