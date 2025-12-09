#ifndef CDI_SOFTCDI_MODULES_HPP
#define CDI_SOFTCDI_MODULES_HPP

#include <cstdint>
#include <span>

inline constexpr uint8_t CIAPDRIV_DATA[] = {
#embed "build/CIAPDRIV"
};
inline constexpr std::span<const uint8_t> CIAPDRIV{CIAPDRIV_DATA};

// inline constexpr uint8_t LAUNCHER_DATA[] = {
// #embed "build/LAUNCHER"
// };
// inline constexpr std::span<const uint8_t> LAUNCHER{LAUNCHER_DATA};

inline constexpr uint8_t PT2_DATA[] = {
#embed "build/PT2"
};
inline constexpr std::span<const uint8_t> PT2{PT2_DATA};

inline constexpr uint8_t VID_DATA[] = {
#embed "build/VID"
};
inline constexpr std::span<const uint8_t> VID{VID_DATA};

#endif // CDI_SOFTCDI_MODULES_HPP
