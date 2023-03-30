#ifndef CDI_CDIDISC_HPP
#define CDI_CDIDISC_HPP

#include "CDIDirectory.hpp"
#include "CDIFile.hpp"

#include <fstream>

struct DiscHeader
{
    uint8_t minute;
    uint8_t second;
    uint8_t sector;
    uint8_t mode; // should be 2 for CD-I tracks
};

struct DiscSubheader
{
    uint8_t fileNumber;
    uint8_t channelNumber;
    uint8_t submode;
    uint8_t codingInformation;
};

/** \struct DiscTime
 * \brief Represent a location on a disc.
 */
struct DiscTime
{
    uint8_t minute; /**< \brief The minute */
    uint8_t second; /**< \brief The second */
    uint8_t sector; /**< \brief The sector */
    uint32_t lsn; /**< \brief The logical sector number. */
    uint32_t pos; /**< \brief The position in the disc file. */
};

enum SubmodeBits : uint8_t
{
    cdieof  = 0b10000000, // End of File
    cdirt   = 0b01000000, // Real Time
    cdiform = 0b00100000, // Form
    cditr   = 0b00010000, // Trigger
    cdid    = 0b00001000, // Data
    cdia    = 0b00000100, // Audio
    cdiv    = 0b00000010, // Video
    cdieor  = 0b00000001, // End of Record
    cdiany  = 0b00001110, // Any type of sector
};

class CDIDisc
{
public:
    std::string m_mainModule;
    std::string m_gameName;

    CDIDisc();
    explicit CDIDisc(const std::string& filename);

    CDIDisc(const CDIDisc&) = delete;
    CDIDisc& operator=(const CDIDisc&) = delete;

    CDIDisc(CDIDisc&&) = default;
    CDIDisc& operator=(CDIDisc&&) = default;

    bool Open(const std::string& filename);
    bool IsOpen() const;
    void Close();
    bool Good();
    DiscTime GetTime();

    CDIFile* GetFile(std::string path);

    bool ExportAudio(const std::string& path);
    bool ExportFiles(const std::string& path);
    bool ExportFileSystem(const std::string& path);
    bool ExportVideo(const std::string& path);
    bool ExportRawVideo(const std::string& path);
    bool ExportSectorsInfo(const std::string& path);

private:
    friend CDIFile;
    friend CDIDirectory;

    std::ifstream m_disc;
    DiscHeader m_header;
    DiscSubheader m_subheader;
    CDIDirectory m_rootDirectory;

    bool LoadFileSystem();

    void UpdateSectorInfo();

    uint32_t Tell();
    bool Seek(const uint32_t offset, std::ios::seekdir direction = std::ios::beg);
    bool GotoLBN(const uint32_t lbn, const uint32_t offset = 0);
    bool GotoNextSector(uint8_t submodeMask = 0);

    bool GetData(uint8_t* dst, uint32_t& size, const bool includeEmptySectors = true);
    bool GetRaw(uint8_t* dst, uint32_t size);
    uint8_t  GetByte();
    uint16_t GetWord();
    uint32_t GetLong();
    std::string GetString(uint16_t length = 128, const char delim = ' ');

    inline bool IsEmptySector() const { return !(m_subheader.submode & cdiany) && !m_subheader.channelNumber && !m_subheader.codingInformation; };
    inline uint16_t GetSectorDataSize() const { return (m_subheader.submode & cdiform) ? 2324 : 2048; }
};

#endif // CDI_CDIDISC_HPP
