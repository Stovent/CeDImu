#ifndef CDI_SOFTCDI_MODULES_HPP
#define CDI_SOFTCDI_MODULES_HPP

#include <cstdint>
#include <span>

inline constexpr uint8_t CIAPDRIV_DATA[] = {
#embed "build/CIAPDRIV"
};
inline constexpr std::span<const uint8_t> CIAPDRIV{CIAPDRIV_DATA};

// #include "../../SoftCDI/include/CSD_450.h"
// #include "../../SoftCDI/include/NVDRV.h"
// #include "../../SoftCDI/include/VIDEO.h"

inline constexpr uint8_t SYSGO_DATA[] = {
#embed "build/SYSGO"
};
inline constexpr std::span<const uint8_t> SYSGO{SYSGO_DATA};

#endif // CDI_SOFTCDI_MODULES_HPP
