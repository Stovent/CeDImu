#ifndef CDI_SOFTCDI_MODULES_HPP
#define CDI_SOFTCDI_MODULES_HPP

#include <cstdint>
#include <span>

inline constexpr uint8_t CDFM_DATA[] = {
#embed "build/cdfm"
};
inline constexpr std::span<const uint8_t> CDFM{CDFM_DATA};

inline constexpr uint8_t CIAPDRIV_DATA[] = {
#embed "build/ciapdriv"
};
inline constexpr std::span<const uint8_t> CIAPDRIV{CIAPDRIV_DATA};

inline constexpr uint8_t CSD_DATA[] = {
#embed "build/csd"
};
inline constexpr std::span<const uint8_t> CSD{CSD_DATA};

inline constexpr uint8_t INIT_DATA[] = {
#embed "build/init"
};
inline constexpr std::span<const uint8_t> INIT{INIT_DATA};

inline constexpr uint8_t LAUNCHER_DATA[] = {
#embed "build/launcher"
};
inline constexpr std::span<const uint8_t> LAUNCHER{LAUNCHER_DATA};

inline constexpr uint8_t PT2_DATA[] = {
#embed "build/pt2"
};
inline constexpr std::span<const uint8_t> PT2{PT2_DATA};

inline constexpr uint8_t PT2DRIV_DATA[] = {
#embed "build/pt2driv"
};
inline constexpr std::span<const uint8_t> PT2DRIV{PT2DRIV_DATA};

inline constexpr uint8_t VID_DATA[] = {
#embed "build/vid"
};
inline constexpr std::span<const uint8_t> VID{VID_DATA};

#endif // CDI_SOFTCDI_MODULES_HPP
