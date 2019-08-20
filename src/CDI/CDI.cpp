#include "CDI.hpp"
#include "../utils.hpp"

CDI::CDI(CeDImu* appp) : rootDirectory(1, "/", 0, 1, 1)
{
    app = appp;
}

CDI::~CDI()
{
    if(disk.is_open())
        disk.close();
}

bool CDI::OpenROM(std::string rom)
{
    if(disk.is_open())
        CloseROM();

    disk.open(rom, std::ios::in | std::ios::binary);
    position = 0;

    if(disk.good())
    {
#ifdef _WIN32
        this->romName = rom.substr(rom.rfind('\\')+1);
        this->romPath = rom.substr(0, rom.rfind('\\')+1);
#else
        this->romName = rom.substr(rom.rfind('/')+1);
        this->romPath = rom.substr(0, rom.rfind('/')+1);
#endif
        LoadFiles();
        this->gameFolder = romPath + gameName + "/";
        return romOpened = true;
    }
    else
        return romOpened = false;
}

void CDI::LoadFiles()
{
    uint32_t pos = disk.tellg();
    char c;
    char s[128];
    uint32_t lbn = 0;

    GotoLBN(16, 148); // go to disk label
    disk.get(c); lbn |= (uint8_t)c << 24; // retrives LBN of path table
    disk.get(c); lbn |= (uint8_t)c << 16;
    disk.get(c); lbn |= (uint8_t)c << 8;
    disk.get(c); lbn |= (uint8_t)c;
    disk.seekg(38, std::ios_base::cur); // reserved

    disk.read(s, 128);
    int i = 127;
    for(; s[i] == ' '; i--);
    gameName.assign(s, i+1);

    disk.seekg(256, std::ios_base::cur); // goto application identifier

    disk.read(s, 128);
    for(i = 127; s[i] == ' '; i--);
    std::string mainModuleName(s, i+1);

    GotoLBN(lbn); //go to path table

    while((disk.tellg() % 2352) < 2072) // read the directories on the whole sector
    {
        uint16_t parent = 0;
        std::string dirname;
        disk.get(c); uint8_t nameSize = c;
		disk.get(c);
        if(nameSize == 0)
            break;
        lbn = 0;
        disk.get(c); lbn |= (uint8_t)c << 24;
        disk.get(c); lbn |= (uint8_t)c << 16;
        disk.get(c); lbn |= (uint8_t)c << 8;
        disk.get(c); lbn |= (uint8_t)c;

        disk.get(c); parent |= (uint8_t)c << 8;
        disk.get(c); parent |= (uint8_t)c;

        for(int i = 0; i < nameSize; i++)
        {
            disk.get(c);
            dirname += (c == 0) ? '/' : c;
        }
        if(!isEven(nameSize))
            disk.get(c);

        if(dirname == "/")
            rootDirectory.LBN = lbn;
    }
    rootDirectory.LoadFiles(disk);
    rootDirectory.LoadSubDirectories(disk);

    if(!rootDirectory.GetFile(mainModule.name, mainModule))
        wxMessageBox("Could not find module " + mainModule.name);

    mainModule.name = mainModuleName;

    disk.seekg(pos);
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

bool CDI::CloseROM()
{
    if(disk.is_open())
        disk.close();

    if(disk.is_open())
        return romOpened = false;
    else
        return romOpened = true;
}

uint16_t CDI::GetWord(const uint32_t& addr, bool stay)
{
    uint32_t pos = disk.tellg();
    disk.seekg(addr);
    char a, b;
    disk.get(a);
    disk.get(b);
    if(!stay)
        disk.seekg(pos);
    return (a << 8) | (uint8_t)b;
}

uint16_t CDI::GetNextWord()
{
    char a, b;
    disk.get(a);
    disk.get(b);
    return (a << 8) | (uint8_t)b;
}

void CDI::SetPosition(const uint32_t& pos)
{
    position = pos;
    disk.seekg(position);
    UpdateSectorInfo();
}

uint32_t CDI::GetPosition()
{
    return position;
}

bool CDI::GotoNextSector(uint8_t submodeMask, bool includingCurrentSector)
{
    position = disk.tellg();
    position -= position % 2352;

    if(submodeMask)
    {
        disk.seekg(position);
        if(includingCurrentSector)
        {
            while(!(subheader.Submode & submodeMask))
            {
                disk.seekg(2352, std::ios_base::seekdir::_S_cur);
                UpdateSectorInfo();
            }
        }
        else
        {
            do
            {
                disk.seekg(2352, std::ios_base::seekdir::_S_cur);
                UpdateSectorInfo();
            } while(!(subheader.Submode & submodeMask));
        }
    }
    else
    {
        disk.seekg(position + 2352);
        UpdateSectorInfo();
    }
    if(disk.good())
        return true;
    else
        return false;
}

bool CDI::GotoLBN(uint32_t lbn, uint32_t offset)
{
    position = lbn * 2352 + 24 + offset;
    disk.seekg(position);
    UpdateSectorInfo();
    if(disk.good())
        return true;
    else
        return false;
}

void CDI::UpdateSectorInfo()
{
    const uint32_t tmp = disk.tellg();
    disk.seekg(tmp - (tmp % 2352) + 12);
    char s[8];
    disk.read(s, 8);

    header.Minutes = convertPBCD(s[0]);
    header.Seconds = convertPBCD(s[1]);
    header.Sectors = convertPBCD(s[2]);
    header.Mode = s[3];
    subheader.FileNumber = s[4];
    subheader.ChannelNumber = s[5];
    subheader.Submode = s[6];
    subheader.CodingInformation = s[7];

    disk.seekg(tmp);
}

bool CDI::IsEmptySector()
{
    if(!(subheader.Submode & 0x0E) && subheader.ChannelNumber == 0 && subheader.CodingInformation == 0)
        return true;
    else
        return false;
}
