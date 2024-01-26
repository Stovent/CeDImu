#ifndef CDI_COMMON_TYPES_HPP
#define CDI_COMMON_TYPES_HPP

#include <cstdint>
#include <span>
#include <string>
#include <string_view>

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

inline const char* BoardsToString(Boards b) noexcept
{
    switch(b)
    {
    case Boards::AutoDetect: return "Auto Detect";
    case Boards::MiniMMC: return "MiniMMC";
    case Boards::Mono1: return "Mono1";
    case Boards::Mono2: return "Mono2";
    case Boards::Mono3: return "Mono3";
    case Boards::Mono4: return "Mono4";
    case Boards::Roboco: return "Roboco";
    case Boards::Fail: return "Fail";
    }
    return "Invalid board type";
}

struct InternalRegister
{
    std::string name;
    uint32_t address;
    uint32_t value;
    std::string disassembledValue;

    InternalRegister(std::string_view name, uint32_t address, uint32_t value, std::string_view disassembledValue)
        : name(name), address(address), value(value), disassembledValue(disassembledValue)
    {}
};

struct RAMBank
{
    std::span<const uint8_t> data;
    uint32_t base;
};

#endif // CDI_COMMON_TYPES_HPP
