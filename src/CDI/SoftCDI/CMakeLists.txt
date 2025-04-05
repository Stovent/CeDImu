option(CEDIMU_REBUILD_SOFTCDI "Whether to re-build SoftCDI headers from sources. If OFF, will use the provided headers without modifications." OFF)

set(SOFTCDI_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/CIAPDRIV.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/CSD_450.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/NVDRV.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/VIDEO.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SYSGO.h
)

if(CEDIMU_REBUILD_SOFTCDI)
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
        ${CMAKE_CURRENT_SOURCE_DIR}/ciapdriv.a
        ${CMAKE_CURRENT_SOURCE_DIR}/csd_450.a
        ${CMAKE_CURRENT_SOURCE_DIR}/nvdrv.a
        ${CMAKE_CURRENT_SOURCE_DIR}/sysgo.a
        ${CMAKE_CURRENT_SOURCE_DIR}/video.a
    )
    set(SET_PATH "set PATH='Z:\\;C:\\BIN\\;'")
    add_custom_command(OUTPUT ${SOFTCDI_HEADERS}
        COMMAND DOSBOX
        ARGS -c "mount c ${OS9C}"
        -c "mount d ${CMAKE_CURRENT_SOURCE_DIR}"
        -c ${SET_PATH}
        -c "d:"
        -c "call .\\build.bat" # call is mandatory otherwise exit is not executed
        -c "exit"
        DEPENDS ${SOFTCDI_SOURCES}
    )
endif()

add_library(SoftCDI INTERFACE ${SOFTCDI_HEADERS})
