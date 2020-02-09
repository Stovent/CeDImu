#include "CDIDisk.hpp"

#include "../utils.hpp"
#include <wx/msgdlg.h>

bool CDIDisk::Open(const std::string& filename)
{
    disk.open(filename, std::ios::in | std::ios::binary);
    return disk.is_open();
}

bool CDIDisk::IsOpen()
{
    return disk.is_open();
}

void CDIDisk::Close()
{
    disk.close();
}

bool CDIDisk::Good()
{
    if(disk.peek() == EOF)
        return false;
    else
        return disk.good();
}

void CDIDisk::Clear()
{
    disk.clear();
}

int CDIDisk::Peek()
{
    return disk.peek();
}

void CDIDisk::UpdateSectorInfo()
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

uint32_t CDIDisk::Tell()
{
    return disk.tellg();
}

void CDIDisk::Seek(const uint32_t offset, std::ios::seekdir direction)
{
    disk.seekg(offset, direction);
    UpdateSectorInfo();
}

bool CDIDisk::GotoLBN(const uint32_t lbn, const uint32_t offset)
{
    uint32_t position = lbn * 2352 + 24 + offset;
    disk.seekg(position);
    UpdateSectorInfo();
    if(disk.good())
        return true;
    else
        return false;
}

/**
* Go to the begining of the data of the next sector (after the subheader)
* @param submodeMask: go to the next sector matching the given mask (only bit 1, 2 and 3 are used).
* @param maskIncludeCurrentSector: specifiy whether the current sector have to be taken in consideration when searching for the next sector with the specified mask.
* @param maxSectorCount: Max number of sector to search for (e.g. to search for all the audio sector in a restricted range, such a single file).
* @param includeAllSectors: specify whether maxSectorCount should include Message sectors and Empty sectors or not.
*
* @return returns false if an error flag is set in the file or if no sector matching the given mask was found in the specified range. Otherwise returns true.
**/
bool CDIDisk::GotoNextSector(uint8_t submodeMask, const bool maskIncludeCurrentSector, uint32_t maxSectorCount, const bool includeAllSectors)
{
    uint32_t position = disk.tellg();
    position -= position % 2352 - 24;

    disk.seekg(position);
    UpdateSectorInfo();

    if(submodeMask &= 0x0E)
    {
        if(!maskIncludeCurrentSector)
            disk.seekg(2352, std::ios::cur);

        while(!(subheader.Submode & submodeMask) && Good())
        {
            if(includeAllSectors)
                maxSectorCount--;
            else
                if(!IsEmptySector())
                    maxSectorCount--;

            if(!maxSectorCount)
                return false;

            disk.seekg(2352, std::ios::cur);
            UpdateSectorInfo();
        }
    }
    else
    {
        do
        {
            disk.seekg(2352, std::ios::cur);
            UpdateSectorInfo();
            --maxSectorCount;
            if(includeAllSectors)
                break;
        } while(IsEmptySector() && Good() && maxSectorCount);
    }

    if(disk.good())
        return true;
    else
        return false;
}

/**
* Get data from sectors only, starting at the current file cursor position
**/
bool CDIDisk::GetData(char* dst, uint32_t size, const bool includeEmptySectors)
{
    uint32_t index = 0;
    while(size)
    {
        uint16_t length = GetSectorDataSize();
        length = (length > size) ? size : length;
        disk.read(&dst[index], length);
        size -= length;
        index += length;
        if(size) // to make sure GetData let the file cursor at the last read data and not at the next sector
            GotoNextSector(0, false, UINT32_MAX, includeEmptySectors);
    }

    if(disk.good())
        return true;
    else
        return false;
}

/**
* Get data from the current file cursor position
**/
bool CDIDisk::Read(char* dst, uint32_t size)
{
    disk.read(dst, size);
    if(disk.good())
        return true;
    else
        return false;
}

/**
* Returns the next byte and
* increasing the file cursor position by 1
**/
uint8_t CDIDisk::GetByte()
{
    char c;
    disk.get(c);
    return c;
}

/**
* Returns the next word value (big endian) and
* increasing the file cursor position by 2
**/
uint16_t CDIDisk::GetWord()
{
    uint16_t var = 0;
    char c;
    disk.get(c); var |= (uint8_t)c << 8;
    disk.get(c); var |= (uint8_t)c;
    return var;
}

/**
* Returns the next long value (big endian) and
* increasing the file cursor position by 4
**/
uint32_t CDIDisk::GetLong()
{
    uint32_t var = 0;
    char c;
    disk.get(c); var |= (uint8_t)c << 24;
    disk.get(c); var |= (uint8_t)c << 16;
    disk.get(c); var |= (uint8_t)c << 8;
    disk.get(c); var |= (uint8_t)c;
    return var;
}

/**
* Reads the next {length} bytes, increasing the file cursor
* position by the same amount, and remove every {delim} characters
* from the end of the string (if any).
**/
std::string CDIDisk::GetString(uint16_t length, const char delim)
{
    char* str = new (std::nothrow) char[length];

    disk.read(str, length);
    for(; str[--length] == delim && length;);

    return std::string(str, length+1);
}

bool CDIDisk::IsEmptySector()
{
    if(!(subheader.Submode & 0x0E) && !subheader.ChannelNumber && !subheader.CodingInformation)
        return true;
    else
        return false;
}

uint16_t CDIDisk::GetSectorDataSize()
{
    return (subheader.Submode & cdiform) ? 2324 : 2048;
}
