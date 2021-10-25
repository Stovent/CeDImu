#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "CDI/common/types.hpp"

#include <string>

namespace Config
{
// Disc
extern std::string discDirectory;

// Board
extern std::string systemBIOS;
extern Boards boardType;
extern bool has32KBNVRAM;
extern std::string initialTime;
extern bool PAL;

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
