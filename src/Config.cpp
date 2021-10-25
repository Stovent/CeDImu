#include "Config.hpp"

#include <fstream>

namespace Config
{
// Disc
std::string discDirectory = "";

// Board
std::string systemBIOS = "";
Boards boardType = Boards::AutoDetect;
bool has32KBNVRAM = false;
std::string initialTime = "599616000"; // empty for current time, 0 for previous time, non-0 for any time.
bool PAL = false;

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
            if(key == "systemBIOS")
                systemBIOS = value;
            else if(key == "discDirectory")
                discDirectory = value;
            else if(key == "boardType")
                boardType = Boards(stoi(value));
            else if(key == "has32KBNVRAM")
                has32KBNVRAM = stoi(value);
            else if(key == "initialTime")
                initialTime = value;
            else if(key == "PAL")
                PAL = stoi(value);
            else if(key == "keyUp")
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
    out << "systemBIOS=" << systemBIOS << std::endl;
    out << "boardType=" << (int)boardType << std::endl;
    out << "has32KBNVRAM=" << has32KBNVRAM << std::endl;
    out << "initialTime=" << initialTime << std::endl;
    out << "PAL=" << PAL << std::endl;

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
