cmake_minimum_required(VERSION 3.0)

if(UNIX)
    set(CMAKE_GENERATOR "Unix Makefiles")
endif()

add_subdirectory(src)
