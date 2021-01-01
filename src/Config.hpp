#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

namespace Config
{
// General
extern std::string systemBIOS;
extern std::string slaveBIOS;
extern std::string ROMDirectory;

// Emulation
extern bool NVRAMUseCurrentTime;
extern bool skipBIOS;

bool saveConfig();
bool loadConfig();
void loadDefaultConfig();

} // namespace Config

#endif // CONFIG_HPP
