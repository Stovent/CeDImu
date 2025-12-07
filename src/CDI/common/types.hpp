#ifndef CDI_COMMON_TYPES_HPP
#define CDI_COMMON_TYPES_HPP

#include <cstdint>
#include <span>
#include <string>
#include <string_view>

/** \brief Specifies the behavior of the memory access functions.
 */
struct BusFlags
{
    bool log : 1; /**< When true, will call the associated log callback. */
};

/** \brief Flags to use for regular data memory accesses in the CPU. */
inline constexpr BusFlags BUS_NORMAL{ .log = true };
/** \brief Flags to use when reading CPU and MCD212 instructions (must not be logged). */
inline constexpr BusFlags BUS_INSTRUCTION{ .log = false };

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

// TODO: move to CDI.hpp
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
};

struct RAMBank
{
    std::span<const uint8_t> data;
    uint32_t base;
};

constexpr uint32_t operator""_KiB(const unsigned long long d) noexcept
{
    return d * 1024;
}

constexpr uint32_t operator""_MiB(const unsigned long long d) noexcept
{
    return d * 1024 * 1024;
}

#endif // CDI_COMMON_TYPES_HPP
