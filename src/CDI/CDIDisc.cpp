#include "CDIDisc.hpp"
#include "common/utils.hpp"

#include <cstring>

CDIDisc::CDIDisc()
    : m_mainModule()
    , m_gameName()
    , m_disc()
    , m_header()
    , m_subheader()
    , m_rootDirectory(1, "/", 0, 1, 1)
{
}

CDIDisc::CDIDisc(const std::string& filename)
    : m_mainModule()
    , m_gameName()
    , m_disc()
    , m_header()
    , m_subheader()
    , m_rootDirectory(1, "/", 0, 1, 1)
{
    Open(filename);
}

/** \brief Open a disc.
 *
 * \param  filename The path to the file to open.
 * \return true if opened successfully, false otherwise.
 */
bool CDIDisc::Open(const std::string& filename)
{
    Close();

    m_disc.open(filename, std::ios::in | std::ios::binary);
    if(!m_disc.is_open())
        return false;

    if(!LoadFileSystem())
    {
        Close();
        return false;
    }

    return true;
}

/** \brief Checks if the disc is opened.
 *
 * \return true if a disc is opened, false otherwise.
 */
bool CDIDisc::IsOpen() const
{
    return m_disc.is_open();
}

/** \brief Close the opened disc.
 */
void CDIDisc::Close()
{
    m_disc.close();
    m_rootDirectory.Clear();
    m_mainModule = "";
    m_gameName = "";
    memset(&m_header, 0, sizeof(m_header));
    memset(&m_subheader, 0, sizeof(m_subheader));
}

/** \brief Check if the disc is readable.
 *
 * \return true if the disc is readable, false otherwise.
 */
bool CDIDisc::Good()
{
    if(m_disc.peek() == EOF)
        return false;

    return m_disc.good();
}

/** \brief Get the location on the disc of the current sector.
 * \return A DiscTime object.
*/
DiscTime CDIDisc::GetTime()
{
    return {
        m_header.minute,
        m_header.second,
        m_header.sector,
        (m_header.minute * 60u * 75u) + ((m_header.second - 2) * 75u) + m_header.sector, // 75 = sectors per second, 60 = seconds per minutes
        Tell(),
    };
}

/** \brief Update header and subheader with the current sector info.
 */
void CDIDisc::UpdateSectorInfo()
{
    const uint32_t tmp = m_disc.tellg();
    m_disc.seekg(tmp - (tmp % 2352) + 12);
    char s[8];
    m_disc.read(s, 8);

    m_header.minute = PBCDToByte(s[0]);
    m_header.second = PBCDToByte(s[1]);
    m_header.sector = PBCDToByte(s[2]);
    m_header.mode = s[3];
    m_subheader.fileNumber = s[4];
    m_subheader.channelNumber = s[5];
    m_subheader.submode = s[6];
    m_subheader.codingInformation = s[7];

    m_disc.seekg(tmp);
}

/** \brief Load every file and directory from the disc.
 * \return true if the main module has been found, false otherwise.
 */
bool CDIDisc::LoadFileSystem()
{
    const uint32_t pos = Tell();

    GotoLBN(16, 148); // go to disc label, Address of path Table

    uint32_t lbn = GetLong();

    Seek(38, std::ios_base::cur); // reserved

    m_gameName = GetString();

    Seek(256, std::ios_base::cur); // go to application identifier

    m_mainModule = GetString();

    GotoLBN(lbn); //go to path table

    while((Tell() % 2352) < 2072) // read the directories on the whole sector
    {
        if(m_disc.fail())
            return false;
        uint8_t nameSize = GetByte();
        if(nameSize == 0)
            break;

        GetByte(); // ignore the next byte

        lbn = GetLong();

        GetWord(); // uint16_t parent =

        std::string dirname = GetString(nameSize);
        if(!isEven(nameSize))
            GetByte();

        if(dirname[0] == '\0')
        {
            m_rootDirectory.LBN = lbn;
        }
    }
    m_rootDirectory.LoadContent(*this);

    Seek(pos);

    if((m_rootDirectory.GetFile(m_mainModule)) == nullptr)
        return false;

    return true;
}

/** \brief Get file from its path on the disc.
 *
 * \param  path The full path from the root directory of the disc to the file.
 * \return A pointer to the file, or nullptr is not found.
 *
 * The path must not start with a '/'.
 * e.g. "CMDS/cdi_gate" for file "cdi_gate" in the "CMDS" folder in the root directory.
 */
CDIFile* CDIDisc::GetFile(std::string path)
{
    return m_rootDirectory.GetFile(path);
}

/** \brief Get the current file cursor position.
 *
 * \return The current file cursor position.
 */
uint32_t CDIDisc::Tell()
{
    return m_disc.tellg();
}

/** \brief Set the file cursor position.
 *
 * \param  offset Offset from the given direction.
 * \param  direction The direction in which the offset will be applied.
 * \return true if the disc is readable, false otherwise.
 */
bool CDIDisc::Seek(const uint32_t offset, std::ios::seekdir direction)
{
    m_disc.clear();
    m_disc.seekg(offset, direction);
    UpdateSectorInfo();
    return Good();
}

/** \brief Go to the given Logical Block Number.
 *
 * \param  lbn The Logical Block Number to go to.
 * \param  offset An optionnal offset from the data section of the sector.
 * \return true if the disc is readable, false otherwise.
 *
 * Set the cursor file position at the beginning of the data section + offset
 * of the logical block number {lbn}.
 */
bool CDIDisc::GotoLBN(const uint32_t lbn, const uint32_t offset)
{
    m_disc.clear();
    m_disc.seekg(lbn * 2352 + 24 + offset);
    UpdateSectorInfo();
    return Good();
}

/** \brief Set the file cursor position at the data section of the next sector.
 *
 * \param  submodeMask See detailed description.
 * \return true if the disc is readable, false otherwise.
 *
 * Go to the next sector which submode matches the given mask.
 * If no mask is provided (or one with 0 as the data, audio and video bits),
 * then only advance to the next sector, without any check.
 * The mask is only applied on the data, audio and video bits of the submode.
 * Use the 'SubmodeBits.cdiany' enum to go to the next non-empty sector.
 */
bool CDIDisc::GotoNextSector(uint8_t submodeMask)
{
    if(submodeMask &= 0x0E)
    {
        do
        {
            m_disc.seekg(2376 - m_disc.tellg() % 2352, std::ios::cur);
            UpdateSectorInfo();
        } while(!(m_subheader.submode & submodeMask) && Good());
    }
    else
    {
        m_disc.seekg(2376 - m_disc.tellg() % 2352, std::ios::cur);
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
 * After execution, {size} is set to the actual data size read from the disc.
 */
bool CDIDisc::GetData(uint8_t* dst, uint32_t& size, const bool includeEmptySectors)
{
    uint32_t index = 0;
    if(!includeEmptySectors && IsEmptySector())
        GotoNextSector(cdiany);
    while(size && m_disc.good())
    {
        uint16_t length = GetSectorDataSize();
        length = (size == 2048) ? length : ((size < length) ? size : length);
        m_disc.read((char*)&dst[index], length);
        index += length;
        size -= (size < 2048) ? size : 2048;
        if(size) // to make sure GetData let the file cursor at the last read data and not at the next sector
            includeEmptySectors ? GotoNextSector() : GotoNextSector(cdiany);
    }
    size = index;
    return m_disc.good();
}

/** \brief Read raw data from the disc.
 *
 * \param  dst The destination buffer.
 * \param  size The size of the data to read.
 * \return true if read successfully, false otherwise.
 *
 * Simply reads the next {size} bytes, without any check.
 */
bool CDIDisc::GetRaw(uint8_t* dst, uint32_t size)
{
    m_disc.read((char*)dst, size);
    return m_disc.good();
}

/** \brief Get the next byte value.
 *
 * \return The byte value read.
 */
uint8_t CDIDisc::GetByte()
{
    char c;
    m_disc.get(c);
    return c;
}

/** \brief Get the word value in big endian format.
 *
 * \return The word value read.
 *
 * Word is a 2-bytes value.
 */
uint16_t CDIDisc::GetWord()
{
    uint16_t var = 0;
    char c;
    m_disc.get(c); var |= (uint8_t)c << 8;
    m_disc.get(c); var |= (uint8_t)c;
    return var;
}

/** \brief Get the next long value in big endian format.
 *
 * \return The long value read.
 *
 * Long is a 4-bytes value.
 */
uint32_t CDIDisc::GetLong()
{
    uint32_t var = 0;
    char c;
    m_disc.get(c); var |= (uint8_t)c << 24;
    m_disc.get(c); var |= (uint8_t)c << 16;
    m_disc.get(c); var |= (uint8_t)c << 8;
    m_disc.get(c); var |= (uint8_t)c;
    return var;
}

/** \brief Get the string at the current file cursor.
 *
 * \param  length The length of the string to read.
 * \param  delim The characters to remove from the end of the string
 * \return The string read from the disc.
 *
 * Reads the next {length} bytes, increasing the file cursor
 * position by the same amount, and remove every {delim} characters
 * from the end of the string (if any).
 * Example: if the string read from the disc is "ABC  " and {delim} is set to ' '
 * then the returned string will be "ABC".
 */
std::string CDIDisc::GetString(uint16_t length, const char delim)
{
    char* str = new (std::nothrow) char[length];

    m_disc.read(str, length);
    for(; str[--length] == delim && length;);

    return std::string(str, length+1);
}
