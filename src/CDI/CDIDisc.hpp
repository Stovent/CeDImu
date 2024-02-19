#ifndef CDI_CDIDISC_HPP
#define CDI_CDIDISC_HPP

#include "CDIDirectory.hpp"
#include "CDIFile.hpp"

#include <fstream>
#include <functional>
#include <span>
#include <string>
#include <string_view>

/** \brief The max number of channels (Green book Appendix II.1.2). */
static constexpr size_t MAX_CHANNEL_NUMBER = 32;
/** \brief The max number of channels on audio sectors (Green book Appendix II.1.2). */
static constexpr size_t MAX_AUDIO_CHANNEL_NUMBER = 16;

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

struct CDISector
{
    DiscHeader header;
    DiscSubheader subheader;
    std::vector<uint8_t> data;

    uint16_t GetSectorDataSize() const { return (subheader.submode & cdiform) ? 2324 : 2048; }
};

/** \brief Encapsulates an ISO of a CD-I disc.
 *
 * This class is not thread-safe.
 *
 * TODO: should I use const and mutable because the disc is actually immutable ?
 */
class CDIDisc
{
public:
    std::string m_mainModule;
    std::string m_gameName;

    CDIDisc();
    explicit CDIDisc(const std::string& filename);

    bool Open(const std::string& filename);
    bool IsOpen() const;
    void Close();
    bool Good();
    DiscTime GetTime();

    const CDIFile* GetFile(std::string path);

    bool ExportAudio(const std::string& path);
    bool ExportFiles(const std::string& path);
    bool ExportFileSystem(const std::string& path);
    bool ExportRawVideo(const std::string& path);
    bool ExportSectorsInfo(const std::string& path);

    void ForEachFile(std::function<void(std::string_view, const CDIFile&)> f);
    void ForEachSector(std::function<void(const CDISector&)> f);

private:
    friend CDIFile;
    friend CDIDirectory;

    std::ifstream m_disc;
    CDISector m_currentSector;
    DiscHeader m_header;
    DiscSubheader m_subheader;
    CDIDirectory m_rootDirectory;

    void UpdateSectorInfo();
    void UpdateCurrentSector();

    bool LoadFileSystem();

    uint32_t Tell();
    bool Seek(const uint32_t offset, std::ios::seekdir direction = std::ios::beg);
    bool GotoLBN(const uint32_t lbn, const uint32_t offset = 0);
    bool GotoNextSector(uint8_t submodeMask = 0);
    bool GotoNextFileSector(uint8_t fileNumber);

    bool GetRaw(std::span<uint8_t> dst);
    uint8_t  GetByte();
    uint16_t GetWord();
    uint32_t GetLong();
    std::string GetString(uint16_t length = 128, const char delim = ' ');

    void ForEachFileSector(uint32_t lbn, std::function<void(const CDISector&)> f);
};

#endif // CDI_CDIDISC_HPP
