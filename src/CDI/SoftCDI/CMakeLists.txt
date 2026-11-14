option(LIBCEDIMU_REBUILD_SOFTCDI "Whether to re-build SoftCDI headers from sources. If OFF, will use the provided headers without modifications." OFF)
message("libCeDImu rebuild SoftCDI: " ${LIBCEDIMU_REBUILD_SOFTCDI})

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(SoftCDI INTERFACE modules.hpp)

if(LIBCEDIMU_REBUILD_SOFTCDI)
    set(OS9C "" CACHE PATH "OS9 compiler root directory")
    message(STATUS "Using OS9C at ${OS9C}")

    set(TOOLCHAIN_CDI "" CACHE PATH "GCC m68k-os9 toolchain root directory")

    find_program(R68
        NAMES m68k-os9-r68 r68
        PATHS ${TOOLCHAIN_CDI}/bin
        REQUIRED
    )
    message(STATUS "found R68 at ${R68}")

    find_program(L68
        NAMES m68k-os9-l68 l68
        PATHS ${TOOLCHAIN_CDI}/bin
        REQUIRED
    )
    message(STATUS "found L68 at ${L68}")

    set(SOFTCDI_SOURCES
        cdfm
        ciapdriv
        csd
        init
        launcher
        pt2
        pt2driv
        vid
    )

    foreach(FILE ${SOFTCDI_SOURCES})
        message(DEBUG "Generating ${FILE}")
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/build/${FILE}
            DEPENDS
                ${CMAKE_CURRENT_SOURCE_DIR}/src/${FILE}.a
                ${CMAKE_CURRENT_SOURCE_DIR}/src/softcdi.d
                ${CMAKE_CURRENT_SOURCE_DIR}/src/softcdi.m
            COMMAND ${R68}
            ARGS
                ${CMAKE_CURRENT_SOURCE_DIR}/src/${FILE}.a
                -o=${CMAKE_CURRENT_SOURCE_DIR}/build/${FILE}.r
                -m0
                -u=${OS9C}/DEFS
                -dialect=c32
                -obj=rof
            COMMAND ${L68}
            ARGS
                ${CMAKE_CURRENT_SOURCE_DIR}/build/${FILE}.r
                -n=${FILE}
                -o=${CMAKE_CURRENT_SOURCE_DIR}/build/${FILE}
                -gu=0.0
                -l=${OS9C}/LIB/SYS.L
                -l=${OS9C}/LIB/CDISYS.L
        )

        list(APPEND SOFTCDI_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/build/${FILE})
    endforeach()

    message(DEBUG "Built ${SOFTCDI_MODULES}")
    add_custom_target(GenerateSoftCDI
        DEPENDS ${SOFTCDI_MODULES}
    )
    add_dependencies(SoftCDI GenerateSoftCDI)
endif()
