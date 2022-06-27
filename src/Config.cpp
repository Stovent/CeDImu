#include "Config.hpp"
#include "CDI/cores/IRTC.hpp"

#include <wx/fileconf.h>

namespace Config
{

const BiosConfig defaultBiosConfig {
    .name = "BIOS config",
    .biosFilePath = "",
    .nvramFileName = "",
    .initialTime = std::to_string(IRTC::defaultTime),
    .boardType = Boards::AutoDetect,
    .PAL = false,
    .has32KbNvram = false,
};

// Disc
std::string discDirectory = "";

// Board
std::vector<BiosConfig> bioses{};

// Controls
int keyUp = 0;
int keyRight = 0;
int keyDown = 0;
int keyLeft = 0;
int key1 = 0;
int key2 = 0;
int key12 = 0;

/** \brief Loads the emulator configuration from a file. "CeDImu.ini".
 *
 * \return true on success, false if something goes wrong.
 *
 * If the file cannot be opened, the emulator will use the default configuration.
 * On Windows the location is "AppData/Roaming/CeDImu.ini". On Linux it is "~/.CeDImu".
 */
bool loadConfig()
{
    wxFileConfig conf("CeDImu");
    wxString str;

    if(!conf.Read("/disc/discPath", &str)) return false;
    discDirectory = str.ToStdString();

    conf.SetPath("/gamepad");
    if(!conf.Read("keyUp", &keyUp)) return false;
    if(!conf.Read("keyRight", &keyRight)) return false;
    if(!conf.Read("keyDown", &keyDown)) return false;
    if(!conf.Read("keyLeft", &keyLeft)) return false;
    if(!conf.Read("key1", &key1)) return false;
    if(!conf.Read("key2", &key2)) return false;
    if(!conf.Read("key12", &key12)) return false;

    conf.SetPath("/bios");
    long lIndexGroup;
    bool hasGroup = conf.GetFirstGroup(str, lIndexGroup);
    while(hasGroup)
    {
        bioses.push_back(defaultBiosConfig); // Adds a new config entry.

        hasGroup = conf.GetNextGroup(str, lIndexGroup);
    }

    size_t index = 0;
    for(BiosConfig& entry : bioses)
    {
        conf.SetPath("/bios/" + std::to_string(index++));

        if(!conf.Read("name", &str)) return false;
        entry.name = str.ToStdString();

        if(!conf.Read("biosFilePath", &str)) return false;
        entry.biosFilePath = str.ToStdString();

        if(!conf.Read("nvramFileName", &str)) return false;
        entry.nvramFileName = str.ToStdString();

        if(!conf.Read("initialTime", &str)) return false;
        entry.initialTime = str.ToStdString();

        int val;
        if(!conf.Read("boardType", &val)) return false;
        entry.boardType = static_cast<Boards>(val);

        if(!conf.Read("PAL", &val)) return false;
        entry.PAL = val;

        if(!conf.Read("has32KbNvram", &val)) return false;
        entry.has32KbNvram = val;
    }

    return true;
}

/** \brief Save the emulator configuration in a file.
 *
 * \return true on success, false if something goes wrong.

 * On Windows the location is "AppData/Roaming/CeDImu.ini". On Linux it is "~/.CeDImu".
 */
bool saveConfig()
{
    wxFileConfig conf("CeDImu");
    conf.DeleteAll();

    if(!conf.Write("/disc/discPath", wxString(discDirectory))) return false;

    conf.SetPath("/gamepad");
    if(!conf.Write("keyUp", keyUp)) return false;
    if(!conf.Write("keyRight", keyRight)) return false;
    if(!conf.Write("keyDown", keyDown)) return false;
    if(!conf.Write("keyLeft", keyLeft)) return false;
    if(!conf.Write("key1", key1)) return false;
    if(!conf.Write("key2", key2)) return false;
    if(!conf.Write("key12", key12)) return false;

    int i = 0;
    for(const BiosConfig& entry : bioses)
    {
        conf.SetPath("/bios/" + std::to_string(i++));

        if(!conf.Write("name", wxString(entry.name))) return false;
        if(!conf.Write("biosFilePath", wxString(entry.biosFilePath))) return false;
        if(!conf.Write("nvramFileName", wxString(entry.nvramFileName))) return false;
        if(!conf.Write("initialTime", wxString(entry.initialTime))) return false;
        if(!conf.Write("boardType", static_cast<int>(entry.boardType))) return false;
        if(!conf.Write("PAL", entry.PAL)) return false;
        if(!conf.Write("has32KbNvram", entry.has32KbNvram)) return false;
    }

    return conf.Flush(); // Technically it saves twice, here and in the dtor, but np I hope.
}

} // namespace Config
