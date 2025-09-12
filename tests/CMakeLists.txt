set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tests
    testVideoDecoders.cpp
)

target_compile_options(tests PRIVATE -Wall -Wextra -pedantic)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain CeDImu)

option(CEDIMU_TESTS_ASAN "Add address sanitizer options for the unit tests" OFF)
message("CeDImu tests sanitize: " ${CEDIMU_TESTS_ASAN})

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") # Common flags for GCC and Clang.
    if(CEDIMU_TESTS_ASAN)
        target_compile_options(tests PRIVATE -fsanitize=address)
        target_link_options(tests PUBLIC -fsanitize=address -static-libasan)
    endif()
endif()
