#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "CDI/common/types.hpp"
#include "CDI/cores/IRTC.hpp"

#include <ctime>
#include <string>
#include <vector>

namespace Config
{

struct BiosConfig
{
    std::string name;
    std::string biosFilePath;
    std::string nvramFileName;
    std::string initialTime;
    Boards boardType;
    bool PAL;
    bool has32KbNvram;
    bool useSoftCDIModules;
};

inline const BiosConfig DEFAULT_BIOS_CONFIG {
    .name = "BIOS config",
    .biosFilePath = "",
    .nvramFileName = "",
    .initialTime = std::to_string(IRTC::DEFAULT_TIME),
    .boardType = Boards::AutoDetect,
    .PAL = false,
    .has32KbNvram = false,
    .useSoftCDIModules = false,
};

// Disc
extern std::string discDirectory;

// BIOSes
extern std::vector<BiosConfig> bioses;

// Controls
extern int keyUp;
extern int keyRight;
extern int keyDown;
extern int keyLeft;
extern int key1;
extern int key2;
extern int key12;

bool saveConfig();
bool loadConfig();

} // namespace Config

#endif // CONFIG_HPP
