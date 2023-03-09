cmake_minimum_required(VERSION 3.13)

project(CeDImu)

if(UNIX)
    set(CMAKE_GENERATOR "Unix Makefiles")
endif()

option(CEDIMU_BUILD_CDITOOL "Build cditool" OFF)

if(CEDIMU_BUILD_CDITOOL)
    add_subdirectory(cditool)
endif()

add_subdirectory(src)
