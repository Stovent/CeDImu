#ifndef CDI_COMMON_ENUMS_HPP
#define CDI_COMMON_ENUMS_HPP

#include <cstdint>

enum BusFlags : uint8_t
{
    NoFlags = 0b00,
    Trigger = 0b01,
    Log     = 0b10,
};

enum class Boards
{
    AutoDetect,
    MiniMMC,
    Mono1,
    Mono2,
    Mono3,
    Mono4,
    Roboco,
    Fail,
};

#endif // CDI_COMMON_ENUMS_HPP
