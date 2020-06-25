#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

namespace Config
{
// General
extern std::string BIOSPath;
extern std::string ROMPath;

// Emulation
extern bool skipBIOS;

bool saveConfig();
bool loadConfig();
void loadDefaultConfig();

} // namespace Config

#endif // CONFIG_HPP
