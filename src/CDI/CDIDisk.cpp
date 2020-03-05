#include "CDIDisk.hpp"

#include "../utils.hpp"
#include <wx/msgdlg.h>

bool CDIDisk::Open(const std::string& filename)
{
    disk.open(filename, std::ios::in | std::ios::binary);
    return disk.is_open();
}

bool CDIDisk::IsOpen() const
{
    return disk.is_open();
}

void CDIDisk::Close()
{
    disk.close();
    memset(&header, 0, sizeof(header));
    memset(&subheader, 0, sizeof(subheader));
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

bool CDIDisk::Seek(const uint32_t offset, std::ios::seekdir direction)
{
    Clear();
    disk.seekg(offset, direction);
    UpdateSectorInfo();
    return Good();
}

/**
* Set the cursor file position at the beginning of the data section + offset
* of the logical block number {lbn}.
**/
bool CDIDisk::GotoLBN(const uint32_t lbn, const uint32_t offset)
{
    Clear();
    disk.seekg(lbn * 2352 + 24 + offset);
    UpdateSectorInfo();
    return Good();
}

/**
* Go to the next sector which submode matches the given mask.
* If no mask is provided (or one with 0 as the data, audio and video bits),
* then only advance to the next sector, without any check.
* The mask is only applied on the data, audio and video bits of the submode.
* Use the 'SubmodeBits.cdiany' enum to go to the next non-empty sector.
**/
bool CDIDisk::GotoNextSector(uint8_t submodeMask)
{
    if(submodeMask &= 0x0E)
    {
        do
        {
            disk.seekg(2376 - disk.tellg() % 2352, std::ios::cur);
            UpdateSectorInfo();
        } while(!(subheader.Submode & submodeMask) && Good());
    }
    else
    {
        disk.seekg(2376 - disk.tellg() % 2352, std::ios::cur);
        UpdateSectorInfo();
    }
    return Good();
}

/**
* Read {size} bytes from sectors only, starting at the current file cursor position.
* If {size} is greater than the sector data size, then it will continue reading on
* the data section of the next sector.
* Returns true if no problems occured, false otherwise.
* After execution, {size} is set to the actual data size read from the disk.
**/
bool CDIDisk::GetData(char* dst, uint32_t& size, const bool includeEmptySectors)
{
    uint32_t index = 0;
    while(size && disk.good())
    {
        uint16_t length = GetSectorDataSize();
        length = (size == 2048) ? length : ((size < length) ? size : length);
        disk.read(&dst[index], length);
        index += length;
        size -= (size < 2048) ? size : 2048;
        if(size) // to make sure GetData let the file cursor at the last read data and not at the next sector
            includeEmptySectors ? GotoNextSector() : GotoNextSector(cdiany);
    }
    size = index;
    return disk.good();
}

/**
* Read {size} bytes from the current file cursor position, increasing it
* by the same amount. Does not perform any check on sector position.
**/
bool CDIDisk::Read(char* dst, uint32_t size)
{
    disk.read(dst, size);
    return disk.good();
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
