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

bool CDI::OpenROM(std::string file, std::string path)
{
    if(disk.is_open())
        CloseROM();

    disk.open(path + file, std::ios::binary);
    position = 0;

    if(disk.good())
    {
        this->romName = file;
        this->romPath = path;
        LoadFiles();
        return true;
    }
    else
        return false;
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
    gameName.assign(s, 128);
    int i = 0; for(i = 126; i >= 0; i--) { if(gameName[i] != ' ') break; }
    if(i != 0) gameName.erase(++i);

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
    disk.seekg(pos);
}

bool CDI::CloseROM()
{
    if(disk.is_open())
        disk.close();

    if(disk.is_open())
        return false;
    else
        return true;
}

uint16_t CDI::GetWord(const uint32_t& addr, bool stay = false)
{
    uint32_t pos = position;
    disk.seekg(addr);
    char a, b;
    disk.get(a);
    disk.get(b);
    if(!stay)
        position = pos+2;
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

bool CDI::GotoNextSector(uint8_t mask)
{
    position = disk.tellg();
    position = position - (position % 2352) + 2376;
    disk.seekg(position);
    UpdateSectorInfo();
    if(mask)
    {
        while(disk.good())
        {
            if(subheader.Submode & mask)
                return true;
            position = position - (position % 2352) + 2376;
            disk.seekg(position);
            UpdateSectorInfo();
        }
    }
    else
        return true;
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
    char byte;
    uint32_t tmp = disk.tellg();
    disk.seekg(tmp - (tmp % 2352) + 12);
    disk.get(byte); header.Minutes = byte;
    disk.get(byte); header.Seconds = byte;
    disk.get(byte); header.Sectors = byte;
    disk.get(byte); header.Mode = byte;
    disk.get(byte); subheader.FileNumber = byte;
    disk.get(byte); subheader.ChannelNumber = byte;
    disk.get(byte); subheader.Submode = byte;
    disk.get(byte); subheader.CodingInformation = byte;
    disk.seekg(tmp);
}

bool CDI::IsEmptySector()
{
    uint32_t tmp = disk.tellg();
    char s[2048];
    disk.seekg(tmp - (tmp % 2352) + 24);
    disk.read(s, 2048);
    disk.seekg(tmp);

    if((subheader.Submode & 0x0E) == 0)
    {
        for(uint16_t i = 0; i < 2048; i++)
            if(s[i] != 0)
                return false;
    }
    else
        return false;
    return true;
}
