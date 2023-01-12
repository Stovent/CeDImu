cmake_minimum_required(VERSION 3.13)

project(useless)

if(UNIX)
    set(CMAKE_GENERATOR "Unix Makefiles")
endif()

option(BUILD_CDITOOL "Build cditool" OFF)

add_subdirectory(src)

if(BUILD_CDITOOL)
    add_subdirectory(cditool)
endif()
