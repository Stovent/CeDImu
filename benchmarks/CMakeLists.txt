# TODO: use a specific benchmarking tool like google benchmark?
cmake_minimum_required(VERSION 3.25)

project(CeDImu-benchmarks)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(benchmarkRenderers benchmarkRenderers.cpp)
target_link_libraries(benchmarkRenderers CeDImu)

add_executable(benchmarkVideoDecoders benchmarkVideoDecoders.cpp)
target_link_libraries(benchmarkVideoDecoders CeDImu)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(benchmarkRenderers PUBLIC -Wall -Wextra -pedantic -march=native)
    target_compile_options(benchmarkVideoDecoders PUBLIC -Wall -Wextra -pedantic -march=native)

    if(WIN32)
        target_link_options(benchmarkRenderers PUBLIC -static-libgcc -static-libstdc++)
        target_link_options(benchmarkVideoDecoders PUBLIC -static-libgcc -static-libstdc++)
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(WIN32)
        # GCC does not support 32-byte aligned stack https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54412
        target_compile_options(benchmarkRenderers PRIVATE -Wa,-muse-unaligned-vector-move)
        target_compile_options(benchmarkVideoDecoders PRIVATE -Wa,-muse-unaligned-vector-move)
    endif()

    # target_compile_options(benchmarkVideoDecoders PRIVATE -fsanitize=address)
    # target_link_options(benchmarkVideoDecoders PUBLIC -fsanitize=address -static-libasan)
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT ipoAvailable)
if(ipoAvailable)
    set_property(TARGET benchmarkRenderers PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_property(TARGET benchmarkVideoDecoders PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
