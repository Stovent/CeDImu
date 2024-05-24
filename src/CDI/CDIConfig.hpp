#ifndef CDI_CDICONFIG_HPP
#define CDI_CDICONFIG_HPP

#include "cores/IRTC.hpp"

#include <ctime>
#include <optional>

/** \struct CDIConfig
 * \brief Configuration of a CDI player.
 */
struct CDIConfig
{
    bool PAL; /**< true for PAL, false for NTSC. */
    std::optional<std::time_t> initialTime; /**< initial time used by the timekeeper, or nullopt to use the stored time. */
    bool has32KBNVRAM; /**< True if the board has 32KB of NVRAM, false for 8KB. */
};

constexpr CDIConfig defaultConfig = {
    true,
    IRTC::defaultTime,
    false,
}; /**< Default configuration used by CDI if no one is provided. */

#endif // CDI_CDICONFIG_HPP
