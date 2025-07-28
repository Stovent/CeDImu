target_sources(CeDImu
    PRIVATE
        IRTC.hpp
        ISlave.hpp
)

add_subdirectory(CDDrive)
add_subdirectory(DS1216)
add_subdirectory(M48T08)
add_subdirectory(MC68HC05)
add_subdirectory(MCD212)
add_subdirectory(SCC68070)
