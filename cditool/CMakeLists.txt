cmake_minimum_required(VERSION 3.0)

project(cditool)

message(VERBOSE "If build fails, make sure libcdio-dev is installed. (sudo apt install libcdio-dev)")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(cditool
    src/cditool.c
    src/import.c
)

target_link_libraries(cditool cdio)
