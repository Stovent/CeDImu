#ifndef CDI_COMMON_ENUMS_HPP
#define CDI_COMMON_ENUMS_HPP

#include <cstdint>

enum BusFlags : uint8_t
{
    NoFlags = 0b00,
    Trigger = 0b01,
    Log     = 0b10,
};

#endif // CDI_COMMON_ENUMS_HPP
