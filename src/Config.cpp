#include "Config.hpp"
#include "CDI/cores/IRTC.hpp"

#include <fstream>

namespace Config
{
// Disc
std::string discDirectory = "";

// Board
std::vector<BiosConfig> bioses = {BiosConfig {
    .name = "Default BIOS",
    .filePath = "",
    .initialTime = std::to_string(IRTC::defaultTime),
    .PAL = false,
    .has32KBNVRAM = false,
    .boardType = Boards::AutoDetect,
}};

// Controls
int keyUp = 0;
int keyRight = 0;
int keyDown = 0;
int keyLeft = 0;
int key1 = 0;
int key2 = 0;
int key12 = 0;

/** \brief Loads the emulator configuration from a file named "CeDImu.ini".
 *
 * \return false if the file could not be opened, true otherwise.
 *
 * The file should be in the same folder as the emulator.
 * If the file cannot be opened, the emulator will use the default configuration.
 */
bool loadConfig()
{
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
            std::string key(line.substr(0, pos)), value(line.substr(pos + 1));
            if(key == "keyUp")
                keyUp = stoi(value);
            else if(key == "keyRight")
                keyRight = stoi(value);
            else if(key == "keyDown")
                keyDown = stoi(value);
            else if(key == "keyLeft")
                keyLeft = stoi(value);
            else if(key == "key1")
                key1 = stoi(value);
            else if(key == "key2")
                key2 = stoi(value);
            else if(key == "key12")
                key12 = stoi(value);
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

    out << "[Disc]" << std::endl;
    out << "discDirectory=" << discDirectory << std::endl;

    out << "[Board]" << std::endl;

    out << "[Controls]" << std::endl;
    out << "keyUp=" << keyUp << std::endl;
    out << "keyRight=" << keyRight << std::endl;
    out << "keyDown=" << keyDown << std::endl;
    out << "keyLeft=" << keyLeft << std::endl;
    out << "key1=" << key1 << std::endl;
    out << "key2=" << key2 << std::endl;
    out << "key12=" << key12 << std::endl;

    out.close();
    return true;
}

} // namespace Config
