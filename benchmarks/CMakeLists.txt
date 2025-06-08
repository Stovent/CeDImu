# TODO: use a specific benchmarking tool like google benchmark?

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(benchmarkRenderers benchmarkRenderers.cpp)
target_link_libraries(benchmarkRenderers CeDImu)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(benchmarkRenderers PUBLIC -Wall -Wextra -pedantic -march=native)
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT ipoAvailable)
if(ipoAvailable)
    set_property(TARGET benchmarkRenderers PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
