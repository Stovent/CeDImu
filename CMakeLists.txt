cmake_minimum_required(VERSION 3.15)

if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif()

project(CeDImu)

option(CEDIMU_BUILD_CDITOOL "Build cditool" OFF)

if(CEDIMU_BUILD_CDITOOL)
    add_subdirectory(cditool)
endif()

add_subdirectory(src)
