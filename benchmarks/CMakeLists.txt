# TODO: use a specific benchmarking tool like google benchmark?

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(benchmarkRenderers benchmarkRenderers.cpp)
target_link_libraries(benchmarkRenderers CeDImu)
