#ifndef FLAGS_HPP
#define FLAGS_HPP

#include <cstdint>

enum BusFlags : uint8_t
{
    NoFlags = 0b00,
    Trigger = 0b01,
    Log     = 0b10,
};

#endif // FLAGS_HPP
