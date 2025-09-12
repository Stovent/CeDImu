cmake_minimum_required(VERSION 3.15)

if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

project(CeDImu)

option(CEDIMU_BUILD_CDITOOL "Build cditool" OFF)
if(CEDIMU_BUILD_CDITOOL)
    add_subdirectory(cditool)
endif()

option(CEDIMU_BENCHMARKS "Compile the benchmarks" OFF)
if(CEDIMU_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

option(CEDIMU_TESTS "Compile the tests" ON)

add_subdirectory(externals)
add_subdirectory(src)

if(CEDIMU_TESTS)
    add_subdirectory(tests)
endif()
