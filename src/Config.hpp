#ifndef CONFIG_HPP
#define CONFIG_HPP

namespace Config
{

extern bool skipBIOS;
extern std::string ROMPath;
extern std::string BIOSPath;

bool saveConfig();
bool loadConfig();
void SetDefaultConfig();

}

#endif // CONFIG_HPP
