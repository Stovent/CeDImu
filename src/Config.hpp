#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "CDI/common/types.hpp"

#include <ctime>
#include <string>
#include <vector>

namespace Config
{

struct BiosConfig
{
    std::string name;
    std::string filePath;
    std::string initialTime;
    bool PAL;
    bool has32KBNVRAM;
    Boards boardType;
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
