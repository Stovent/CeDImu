#ifndef CDICONFIG_HPP
#define CDICONFIG_HPP

#include "cores/IRTC.hpp"

#include <ctime>

/** \struct CDIConfig
 * \brief Configuration of a CDI player.
 */
struct CDIConfig
{
    bool PAL; /**< true for PAL, false for NTSC. */
    std::time_t initialTime; /**< initial time used by the timekeeper. */
    bool has32KBNVRAM; /**< True if the board has 32KB of NVRAM, false for 8KB. */
};

constexpr CDIConfig defaultConfig = {
    .PAL = true,
    .initialTime = IRTC::defaultTime,
    .has32KBNVRAM = false,
}; /**< Default configuration used by CDI if no one is provided. */

#endif // CDICONFIG_HPP
