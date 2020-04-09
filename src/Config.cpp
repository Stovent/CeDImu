#include <fstream>

#include "Config.hpp"

namespace Config
{
// General
std::string ROMPath;
std::string BIOSPath;

// Emulation
bool skipBIOS;

bool loadConfig()
{
    std::ifstream in("CeDImu.ini");
    if(!in.is_open())
        return false;

    while(in.good())
    {
        std::string line;
        std::getline(in, line);
        size_t pos;
        if((pos = line.find('=')) != std::string::npos)
        {
            std::string key(line.substr(0, pos)), value(line.substr(pos+1));
            if(key == "skipBIOS")
                skipBIOS = stoi(value);
            else if(key == "ROMPath")
                ROMPath = value;
            else if(key == "BIOSPath")
                BIOSPath = value;
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
    out << "ROMPath=" << ROMPath << std::endl;
    out << "BIOSPath=" << BIOSPath << std::endl;

    out << "[Emulation]" << std::endl;
    out << "skipBIOS=" << skipBIOS << std::endl;

    out.close();
    return true;
}

void SetDefaultConfig()
{
    skipBIOS = false;
}

} // namespace Config
