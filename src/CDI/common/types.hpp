#ifndef CDI_COMMON_TYPES_HPP
#define CDI_COMMON_TYPES_HPP

#include <cstdint>
#include <string>

enum BusFlags
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

struct InternalRegister
{
    std::string name;
    uint32_t address;
    uint32_t value;
    std::string disassembledValue;
};

struct RAMBank
{
    const uint8_t* data;
    uint32_t base;
    uint32_t size;
};

#endif // CDI_COMMON_TYPES_HPP
