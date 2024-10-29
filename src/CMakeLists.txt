cmake_minimum_required(VERSION 3.24) # CMake 3.24 fixes wxWidgets detection.

find_package(wxWidgets REQUIRED COMPONENTS aui adv core base)

if(NOT wxWidgets_FOUND)
    message(FATAL_ERROR "wxWidgets not found")
endif()

option(CEDIMU_ENABLE_LTO "Enable LTO if available" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    SET(CMAKE_CXX_FLAGS_PROFILERELEASE
        "-O3 -pg -DwxDEBUG_LEVEL=0"
        CACHE STRING "Flags used by the C++ compiler during profile builds with release optimisations."
        FORCE
    )
endif()

set(CEDIMU_ENABLE_LOG ON)

add_subdirectory(CDI)

add_executable(CeDImu-GUI WIN32
    CeDImu.cpp
    CeDImu.hpp
    Config.cpp
    Config.hpp
    export.cpp
    export.hpp
    main.cpp
)
add_subdirectory(GUI)

set_target_properties(CeDImu-GUI PROPERTIES OUTPUT_NAME "CeDImu")

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" ) # TODO: check if CMAKE_CONFIGURATION_TYPES also contains "Release".
    target_compile_definitions(CeDImu-GUI PUBLIC wxDEBUG_LEVEL=0)
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
    target_compile_options(CeDImu-GUI PRIVATE -Wall -Wextra -Wpedantic)
#    target_compile_options(CeDImu-GUI PRIVATE -Wmaybe-uninitialized -Wduplicated-cond -Wlogical-op -Wmisleading-indentation -Wreturn-type -Wold-style-cast -Wnon-virtual-dtor -Woverloaded-virtual)
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
