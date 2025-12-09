option(LIBCEDIMU_REBUILD_SOFTCDI "Whether to re-build SoftCDI headers from sources. If OFF, will use the provided headers without modifications." OFF)
message("libCeDImu rebuild SoftCDI: " ${LIBCEDIMU_REBUILD_SOFTCDI})

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(SoftCDI INTERFACE modules.hpp)

if(LIBCEDIMU_REBUILD_SOFTCDI)
    set(OS9C "" CACHE PATH "OS9 compiler root directory")

    find_program(DOSBOX
        NAMES dosbox dosbox.exe
        REQUIRED
    )
    message(STATUS "found DOSBOX at ${DOSBOX}")

    find_program(R68
        NAMES R68.EXE
        PATHS ${OS9C}/BIN
        REQUIRED
    )
    message(STATUS "found R68 at ${R68}")

    set(SOFTCDI_SOURCES
        src/ciapdriv.a
        # src/launcher.a
        src/pt2.a
        src/softcdi.d
        src/softcdi.m
        # csd_450.a
        # nvdrv.a
        src/vid.a
        # video.a
    )

    set(SOFTCDI_MODULES
        build/CIAPDRIV
        # build/LAUNCHER
        build/PT2
        # build/CSD_450
        # build/NVDRV
        build/VID
        # build/VIDEO
    )

    set(SET_DOS_PATH "set PATH='Z:\\;C:\\BIN\\;'")
    add_custom_target(GenerateSoftCDI
        DOSBOX
        -noconsole
        -c "mount c ${OS9C}"
        -c "mount d ${CMAKE_CURRENT_SOURCE_DIR}"
        -c ${SET_DOS_PATH}
        -c "d:"
        -c "call .\\build.bat" # call is mandatory otherwise exit is not executed
        -c "exit"

        DEPENDS ${SOFTCDI_SOURCES}
        BYPRODUCTS ${SOFTCDI_MODULES}
    )
    add_dependencies(SoftCDI GenerateSoftCDI)
endif()
