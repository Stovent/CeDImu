#include "Config.hpp"

#include <fstream>

namespace Config
{
// General
std::string BIOSPath;
std::string ROMPath;

// Emulation
bool skipBIOS;
bool limitFPS;

bool loadConfig()
{
    loadDefaultConfig();
    std::ifstream in("CeDImu.ini");
    if(!in.is_open())
        return false;

    std::string line;
    while(in.good())
    {
        std::getline(in, line);
        size_t pos;
        if((pos = line.find('=')) != std::string::npos)
        {
            std::string key(line.substr(0, pos)), value(line.substr(pos+1));
            if(key == "BIOSPath")
                BIOSPath = value;
            else if(key == "ROMPath")
                ROMPath = value;
            else if(key == "skipBIOS")
                skipBIOS = stoi(value);
            else if(key == "limitFPS")
                limitFPS = stoi(value);
        }
    }

    in.close();
    return true;
}

bool saveConfig()
{
    std::ofstream out("CeDImu.ini");
    if(!out.is_open())
        return false;

    out << "[General]" << std::endl;
    out << "BIOSPath=" << BIOSPath << std::endl;
    out << "ROMPath=" << ROMPath << std::endl;

    out << "[Emulation]" << std::endl;
    out << "skipBIOS=" << skipBIOS << std::endl;
    out << "limitFPS=" << limitFPS << std::endl;

    out.close();
    return true;
}

void loadDefaultConfig()
{
    BIOSPath = "";
    ROMPath = "";
    skipBIOS = false;
    limitFPS = true;
}

} // namespace Config
