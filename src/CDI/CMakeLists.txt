cmake_minimum_required(VERSION 3.25)

# set(CMAKE_CXX_CLANG_TIDY
#     clang-tidy;
#     -header-filter=.;
#     -checks=bugprone-*,cert-*,clang-analyzer-*,concurrency-*,cppcoreguidelines-*,misc-*,modernize-*,-modernize-use-trailing-return-type;)
    # -checks=bugprone-*,cert-*,clang-analyzer-*,concurrency-*,cppcoreguidelines-*,misc-*,modernize-*,
    # performance-*,portability-*,readability-*;)

project(libCeDImu)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(LIBCEDIMU_ENABLE_RENDERERSIMD "Enable the SIMD renderer, requires headers <simd> or <experimental/simd>" OFF)
message("libCeDImu enable RendererSIMD: " ${LIBCEDIMU_ENABLE_RENDERERSIMD})
option(LIBCEDIMU_PROFILE_GNU "Add profile options for GNU compiler" OFF)
message("libCeDImu profile GNU: " ${LIBCEDIMU_PROFILE_GNU})
option(LIBCEDIMU_ENABLE_ASAN "Add address sanitizer options" OFF)
message("libCeDImu sanitize: " ${LIBCEDIMU_ENABLE_ASAN})

add_library(CeDImu ${LIBRARY_TYPE}
    CDI.cpp
    CDI.hpp
    CDIConfig.hpp
    CDIDirectory.cpp
    CDIDirectory.hpp
    CDIDisc.cpp
    CDIDisc.hpp
    CDIFile.cpp
    CDIFile.hpp
    Export.cpp
    PointingDevice.cpp
    PointingDevice.hpp
)
target_include_directories(CeDImu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(LIBCEDIMU_ENABLE_RENDERERSIMD)
    target_compile_definitions(CeDImu PUBLIC LIBCEDIMU_ENABLE_RENDERERSIMD)
endif()

add_subdirectory(boards)
add_subdirectory(common)
add_subdirectory(cores)
add_subdirectory(HLE)
add_subdirectory(OS9)
add_subdirectory(SoftCDI)
add_subdirectory(Video)

target_link_libraries(CeDImu PUBLIC SoftCDI)

option(LIBCEDIMU_ENABLE_LOG "Allows the library to print some messages in the console and the use of OnLogMemoryAccess callback" ON)
if(LIBCEDIMU_ENABLE_LOG)
    target_compile_definitions(CeDImu PUBLIC LIBCEDIMU_ENABLE_LOG)
endif()

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") # Common flags for GCC and Clang.
    target_compile_options(CeDImu PRIVATE -Wall -Wextra -pedantic -march=native)
    # target_compile_options(CeDImu PRIVATE -Wmaybe-uninitialized -Wduplicated-cond -Wlogical-op -Wmisleading-indentation -Wreturn-type -Wold-style-cast -Wnon-virtual-dtor -Woverloaded-virtual)

    if(LIBCEDIMU_ENABLE_ASAN)
        target_compile_options(CeDImu PRIVATE -fsanitize=address)
        target_link_options(CeDImu PUBLIC -fsanitize=address -static-libasan)
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(LIBCEDIMU_PROFILE_GNU)
        target_compile_options(CeDImu PRIVATE -pg)
        target_link_options(CeDImu PUBLIC -pg -no-pie)
    endif()

    if(WIN32)
        # GCC does not support 32-byte aligned stack https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54412
        target_compile_options(CeDImu PRIVATE -Wa,-muse-unaligned-vector-move)
    endif()

    target_compile_options(CeDImu PRIVATE -Wnrvo)
    target_link_libraries(CeDImu PRIVATE stdc++fs stdc++exp)
# elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
endif()
