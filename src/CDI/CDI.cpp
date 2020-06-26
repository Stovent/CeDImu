#include "CDI.hpp"

#ifdef USE_STD_FILESYSTEM
#include <filesystem>
#else
#include <wx/filefn.h>
#endif // USE_STD_FILESYSTEM

#include <wx/msgdlg.h>

#include "../utils.hpp"

CDI::CDI() : disk(), rootDirectory(1, "/", 0, 1, 1)
{

}

CDI::~CDI()
{
    CloseROM();
}

/** \brief Open the given ROM.
 *
 * \param  rom Path to the ROM to open.
 * \return true if the ROM have been openend successfully, false otherwise.
 */
bool CDI::OpenROM(const std::string& rom)
{
    CloseROM();

    if(!disk.Open(rom))
        return false;

#ifdef _WIN32
    romPath = rom.substr(0, rom.rfind('\\')+1);
#else
    romPath = rom.substr(0, rom.rfind('/')+1);
#endif

    LoadCDIFileSystem();
    gameFolder = romPath + gameName + "/";
    return true;
}


/** \brief Close the opened ROM.
 */
void CDI::CloseROM()
{
    rootDirectory.Clear();
    if(disk.IsOpen())
        disk.Close();
    mainModule = "";
    gameName = "";
    romPath = "";
    gameFolder = "";
}


/** \brief Load every file and directory from the disk.
 */
void CDI::LoadCDIFileSystem()
{
    const uint32_t pos = disk.Tell();

    disk.GotoLBN(16, 148); // go to disk label, Address of path Table

    uint32_t lbn = disk.GetLong();

    disk.Seek(38, std::ios_base::cur); // reserved

    gameName = disk.GetString();

    disk.Seek(256, std::ios_base::cur); // goto application identifier

    mainModule = disk.GetString();

    disk.GotoLBN(lbn); //go to path table

    while((disk.Tell() % 2352) < 2072) // read the directories on the whole sector
    {
        uint8_t nameSize = disk.GetByte();
        if(nameSize == 0)
            break;

		disk.GetByte(); // ignore the next byte

        lbn = disk.GetLong();

        /*uint16_t parent = */disk.GetWord();

        std::string dirname = disk.GetString(nameSize);
        if(!isEven(nameSize))
            disk.GetByte();

        if(dirname[0] == '\0')
        {
            rootDirectory.dirLBN = lbn;
        }
    }
    rootDirectory.LoadContent(disk);

    if((rootDirectory.GetFile(mainModule)) == nullptr)
        wxMessageBox("Could not find main module " + mainModule);

    disk.Seek(pos);
}

/** \brief Create subdirectories inside the game folder.
 *
 * \param  path The directories to create, separated by '/'.
 * \return false if a folder could not be created, true otherwise.
 *
 * The game folder is the directory where the ROM is located +
 * the game name inside the ROM.
 * Path must not start with an '/' and must end with an '/'.
 * An empty string only creates the game folder only (romPath + gameName).
 * Example: if the game is Alien Gate, and the ROM is in C:/ROMs/
 * then sending path = "files/CMDS/" will create C:/ROMs/Alien Gate/files/CMDS/
 */
bool CDI::CreateSubfoldersFromROMDirectory(std::string path)
{
    std::string newFolder(gameFolder);
    do
    {
#ifdef USE_STD_FILESYSTEM
        if(!std::filesystem::create_directory(newFolder))
            return false;
#else
        if(!wxDirExists(newFolder))
            if(!wxMkdir(newFolder))
                return false;
#endif // USE_STD_FILESYSTEM

        uint32_t pos = path.find('/');
        newFolder += path.substr(0, pos+1);
        path = path.substr(pos+1);
    } while(path.length() > 1);

    return true;
}

/** \brief Get file from its path on the disk.
 *
 * \param  path The full path from the root directory of the disk to the file.
 * \return A pointer to the file, or nullptr is not found.
 *
 * The path must not start with a '/'.
 * e.g. "CMDS/cdi_gate" for file "cdi_gate" in the "CMDS" folder in the root directory.
 */
CDIFile* CDI::GetFile(std::string path)
{
    return rootDirectory.GetFile(path);
}
