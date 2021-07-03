#ifndef CDI_COMMON_ENUMS_HPP
#define CDI_COMMON_ENUMS_HPP

#include <cstdint>
#include <string>

enum BusFlags : uint8_t
{
    NoFlags = 0b00,
    Trigger = 0b01,
    Log     = 0b10,
};

/** \struct LogMemoryAccess
 */
struct LogMemoryAccess
{
    std::string location; /**< CPU, RAM, VDSC, Slave, RTC, etc. */
    std::string direction; /**< Get or Set. */
    std::string size; /**< Byte, Word or Long. */
    uint32_t pc; /**< Program Counter when the access occured. */
    uint32_t address; /**< The bus address. */
    uint32_t data; /**< The data. */
};

#endif // CDI_COMMON_ENUMS_HPP
