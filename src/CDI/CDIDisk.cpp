#include "CDIDisk.hpp"

#include "../utils.hpp"
#include <wx/msgdlg.h>

/** \brief Open a disk.
 *
 * \param  filename The path to the file to open.
 * \return true if opened successfully, false otherwise.
 */
bool CDIDisk::Open(const std::string& filename)
{
    disk.open(filename, std::ios::in | std::ios::binary);
    return disk.is_open();
}

/** \brief Checks if the disk is opened.
 *
 * \return true if a disk is opened, false otherwise.
 */
bool CDIDisk::IsOpen() const
{
    return disk.is_open();
}

/** \brief Close the opened disk.
 */
void CDIDisk::Close()
{
    disk.close();
    memset(&header, 0, sizeof(header));
    memset(&subheader, 0, sizeof(subheader));
}

/** \brief Check if the disk is readable.
 *
 * \return true if the disk is readable, false otherwise.
 */
bool CDIDisk::Good()
{
    if(disk.peek() == EOF)
        return false;
    else
        return disk.good();
}

/** \brief Clear the status bits of the file.
 */
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

/** \brief Get the current file cursor position.
 *
 * \return The current file cursor position.
 */
uint32_t CDIDisk::Tell()
{
    return disk.tellg();
}

/** \brief Set the file cursor position.
 *
 * \param  offset Offset from the given direction.
 * \param  direction The direction in which the offset will be applied.
 * \return true if the disk is readable, false otherwise.
 */
bool CDIDisk::Seek(const uint32_t offset, std::ios::seekdir direction)
{
    Clear();
    disk.seekg(offset, direction);
    UpdateSectorInfo();
    return Good();
}

/** \brief Go to the given Logical Block Number.
 *
 * \param  lbn The Logical Block Number to go to.
 * \param  offset An optionnal offset from the data section of the sector.
 * \return true if the disk is readable, false otherwise.
 *
 * Set the cursor file position at the beginning of the data section + offset
 * of the logical block number {lbn}.
 */
bool CDIDisk::GotoLBN(const uint32_t lbn, const uint32_t offset)
{
    Clear();
    disk.seekg(lbn * 2352 + 24 + offset);
    UpdateSectorInfo();
    return Good();
}

/** \brief Set the file cursor position at the data section of the next sector.
 *
 * \param  submodeMask See detailed description.
 * \return true if the disk is readable, false otherwise.
 *
 * Go to the next sector which submode matches the given mask.
 * If no mask is provided (or one with 0 as the data, audio and video bits),
 * then only advance to the next sector, without any check.
 * The mask is only applied on the data, audio and video bits of the submode.
 * Use the 'SubmodeBits.cdiany' enum to go to the next non-empty sector.
 */
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

/** \brief Read data from the data section of the sectors.
 *
 * \param  dst The destination buffer.
 * \param  size See detailed description.
 * \param  includeEmptySectors If true, empty sectors will not be read and will not count in size.
 * \return true if successfully read, false otherwise.
 *
 * Read {size} bytes from sectors only, starting at the current file cursor position.
 * If {size} is greater than the sector data size, then it will continue reading on
 * the data section of the next sector.
 * Returns true if no problems occured, false otherwise.
 * After execution, {size} is set to the actual data size read from the disk.
 */
bool CDIDisk::GetData(char* dst, uint32_t& size, const bool includeEmptySectors)
{
    uint32_t index = 0;
    if(!includeEmptySectors && IsEmptySector())
        GotoNextSector(cdiany);
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

/** \brief Read raw data from the disk.
 *
 * \param  dst The destination buffer.
 * \param  size The size of the data to read.
 * \return true if read successfully, false otherwise.
 *
 * Simply reads the next {size} bytes, without any check.
 */
bool CDIDisk::Read(char* dst, uint32_t size)
{
    disk.read(dst, size);
    return disk.good();
}

/** \brief Get the next byte value.
 *
 * \return The byte value read.
 */
uint8_t CDIDisk::GetByte()
{
    char c;
    disk.get(c);
    return c;
}

/** \brief Get the word value in big endian format.
 *
 * \return The word value read.
 *
 * Word is a 2-bytes value.
 */
uint16_t CDIDisk::GetWord()
{
    uint16_t var = 0;
    char c;
    disk.get(c); var |= (uint8_t)c << 8;
    disk.get(c); var |= (uint8_t)c;
    return var;
}

/** \brief Get the next long value in big endian format.
 *
 * \return The long value read.
 *
 * Long is a 4-bytes value.
 */
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

/** \brief Get the string at the current file cursor.
 *
 * \param  length The length of the string to read.
 * \param  delim The characters to remove from the end of the string
 * \return The string read from the disk.
 *
 * Reads the next {length} bytes, increasing the file cursor
 * position by the same amount, and remove every {delim} characters
 * from the end of the string (if any).
 * Example: if the string read from the disk is "ABC  " and {delim} is set to ' '
 * then the returned string will be "ABC".
 */
std::string CDIDisk::GetString(uint16_t length, const char delim)
{
    char* str = new (std::nothrow) char[length];

    disk.read(str, length);
    for(; str[--length] == delim && length;);

    return std::string(str, length+1);
}
