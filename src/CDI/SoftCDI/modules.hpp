#ifndef CDI_SOFTCDI_MODULES_HPP
#define CDI_SOFTCDI_MODULES_HPP

#include <cstdint>
#include <span>

constexpr uint8_t CIAPDRIV_DATA[] = {
#embed "build/CIAPDRIV"
};
constexpr std::span<const uint8_t> CIAPDRIV{CIAPDRIV_DATA};

// #include "../../SoftCDI/include/CSD_450.h"
// #include "../../SoftCDI/include/NVDRV.h"
// #include "../../SoftCDI/include/VIDEO.h"

constexpr uint8_t SYSGO_DATA[] = {
#embed "build/SYSGO"
};
constexpr std::span<const uint8_t> SYSGO{SYSGO_DATA};

#endif // CDI_SOFTCDI_MODULES_HPP
