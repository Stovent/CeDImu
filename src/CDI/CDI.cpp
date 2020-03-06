#include "CDI.hpp"
#include "../utils.hpp"

CDI::CDI(CeDImu* app) : disk(), rootDirectory(1, "/", 0, 1, 1)
{
    cedimu = app;
}

CDI::~CDI()
{
    CloseROM();
}

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
    cedimu->mainFrame->SetTitle(gameName + " - CeDImu");
    return true;
}

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

        if(dirname == '\0')
        {
            rootDirectory.dirLBN = lbn;
        }
    }
    rootDirectory.LoadContent(disk);

    if((rootDirectory.GetFile(mainModule)) == nullptr)
        wxMessageBox("Could not find main module " + mainModule);

    disk.Seek(pos);
}

/**
*   path must not start with an '/' and must end with an '/'
*   an empty string only creates the game folder (romPath + gameName)
**/
bool CDI::CreateSubfoldersFromROMDirectory(std::string path)
{
    std::string newFolder(gameFolder);
    do
    {
        if(!wxDirExists(newFolder))
            if(!wxMkdir(newFolder))
                return false;

        uint32_t pos = path.find('/');
        newFolder += path.substr(0, pos+1);
        path = path.substr(pos+1);
    } while(path.length() > 1);

    return true;
}

/**
* Returns a pointer to the file named {name}, or nullptr is not found
**/
CDIFile* CDI::GetFile(std::string name)
{
    return rootDirectory.GetFile(name);
}
