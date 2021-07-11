#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "CDI/CDI.hpp"

#include <string>

namespace Config
{
// General
extern std::string systemBIOS;
extern std::string ROMDirectory;
extern Boards boardType;
extern bool has32KBNVRAM;

// Emulation
extern std::string initialTime;
extern bool PAL;
extern bool skipBIOS;

bool saveConfig();
bool loadConfig();
void loadDefaultConfig();

} // namespace Config

#endif // CONFIG_HPP
