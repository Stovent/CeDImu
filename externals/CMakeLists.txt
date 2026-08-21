if(CEDIMU_TESTS)
    add_subdirectory(Catch2)
endif()

if(LIBCEDIMU_USE_M68000)
    include(m68000.cmake)
endif()
