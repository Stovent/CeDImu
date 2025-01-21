#ifndef CDI_COMMON_TYPES_HPP
#define CDI_COMMON_TYPES_HPP

#include <cstdint>
#include <span>
#include <string>
#include <string_view>

/** \brief Specifies the behavior of the memory access functions.
 * BusFlags.trigger allows modifications of the device and its memory, i.e. when it is false, the device must absolutely not be modified.
 * When BusFlags.trigger is false, acts as a peek memory, so reads have no side effects (only observe the memory),
 * and writes have no effects on some regions.
 */
struct BusFlags
{
    // bool peek : 1; /**< . */
    bool trigger : 1; /**< When true, memory accesses that have side effects are triggered (like reset a flag in a peripheral). */
    bool log : 1; /**< When true, will call the associated log callback. */
};

/** \brief Flags to use for regular memory accesses in the CPU. */
inline constexpr BusFlags BUS_NORMAL{ .trigger = true, .log = true };
/** \brief Flags to use when reading CPU and MCD212 instructions (must not be logged). */
inline constexpr BusFlags BUS_INSTRUCTION{ .trigger = true, .log = false };
/** \brief Flags to use when peeking memory (observing it outside of emulation). */
inline constexpr BusFlags BUS_PEEK{ .trigger = false, .log = false };

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

constexpr const char* BoardsToString(Boards b) noexcept
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
