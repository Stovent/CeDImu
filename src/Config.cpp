#include "Config.hpp"

#include <fstream>

namespace Config
{
// General
std::string systemBIOS;
std::string ROMDirectory;

// Emulation
std::string initialTime; // empty for current time, 0 for previous time, non-0 for any time.
bool PAL;
bool skipBIOS;

/** \brief Loads the emulator configuration from a file named "CeDImu.ini".
 *
 * \return false if the file could not be opened, true otherwise.
 *
 * The file should be in the same folder as the emulator.
 * If the file cannot be opened, the emulator will use the default configuration.
 */
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
            if(key == "systemBIOS")
                systemBIOS = value;
            else if(key == "ROMDirectory")
                ROMDirectory = value;
            else if(key == "initialTime")
                initialTime = value;
            else if(key == "PAL")
                PAL = stoi(value);
            else if(key == "skipBIOS")
                skipBIOS = stoi(value);
        }
    }

    in.close();
    return true;
}

/** \brief Save the emulator configuration in a file named "CeDImu.ini".
 *
 * \return false if the file could not be written, true otherwise.
 */
bool saveConfig()
{
    std::ofstream out("CeDImu.ini");
    if(!out.is_open())
        return false;

    out << "[General]" << std::endl;
    out << "systemBIOS=" << systemBIOS << std::endl;
    out << "ROMDirectory=" << ROMDirectory << std::endl;

    out << "[Emulation]" << std::endl;
    out << "initialTime=" << initialTime << std::endl;
    out << "PAL=" << PAL << std::endl;
    out << "skipBIOS=" << skipBIOS << std::endl;

    out.close();
    return true;
}

/** \brief Loads the default emulator configuration.
 */
void loadDefaultConfig()
{
    systemBIOS = "";
    ROMDirectory = "";
    initialTime = "599616000";
    PAL = false;
    skipBIOS = false;
}

} // namespace Config
