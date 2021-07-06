#ifndef CDICONFIG_HPP
#define CDICONFIG_HPP

#include <ctime>

/** \struct CDIConfig
 * \brief Configuration of a CDI player.
*/
struct CDIConfig
{
    bool PAL; /**< true for PAL, false for NTSC */
    std::time_t initialTime; /**< initial time used by the timekeeper */
    bool has32KBNVRAM; /**< Modified internally, not meant to be set by the user. */
};

extern const CDIConfig defaultConfig; /**< Default configuration used by CDI if no one is provided. */

#endif // CDICONFIG_HPP
