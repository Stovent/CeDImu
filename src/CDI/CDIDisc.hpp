#ifndef CDIDISC_HPP
#define CDIDISC_HPP

#include "CDIDirectory.hpp"
#include "CDIFile.hpp"

#include <fstream>

struct CDIHeader
{
    uint8_t minutes;
    uint8_t seconds;
    uint8_t sectors;
    uint8_t mode; // should be 2 for CD-I tracks
};

struct CDISubheader
{
    uint8_t fileNumber;
    uint8_t channelNumber;
    uint8_t submode;
    uint8_t codingInformation;
};

enum SubmodeBits
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
    std::ifstream disc;

    void UpdateSectorInfo();
    bool CreateSubfoldersFromROMDirectory(std::string path = "");

public:
    CDIHeader header;
    CDISubheader subheader;
    CDIDirectory rootDirectory;
    std::string mainModule;
    std::string gameName;
    std::string romPath;
    std::string gameFolder; // romPath + gameName + "/"

    CDIDisc() : disc(), header(), subheader(), rootDirectory(1, "/", 0, 1, 1) {}
    CDIDisc(const CDIDisc&) = delete;
    CDIDisc(const CDIDisc&&) = delete;

    bool Open(const std::string& filename);
    bool IsOpen() const;
    void Close();
    bool Good();
    void Clear();

    void LoadFileSystem();
    CDIFile* GetFile(std::string path);

    uint32_t Tell();
    bool Seek(const uint32_t offset, std::ios::seekdir direction = std::ios::beg);
    bool GotoLBN(const uint32_t lbn, const uint32_t offset = 0);
    bool GotoNextSector(uint8_t submodeMask = 0);

    bool GetData(char* dst, uint32_t& size, const bool includeEmptySectors = true);
    bool GetRaw(char* dst, uint32_t size);
    uint8_t  GetByte();
    uint16_t GetWord();
    uint32_t GetLong();
    std::string GetString(uint16_t length = 128, const char delim = ' ');

    bool ExportAudio();
    bool ExportFiles();
    void ExportFileSystem();
    bool ExportVideo();
    void ExportSectorsInfo();

    inline bool IsEmptySector() const { return !(subheader.submode & cdiany) && !subheader.channelNumber && !subheader.codingInformation; };
    inline uint16_t GetSectorDataSize() const { return (subheader.submode & cdiform) ? 2324 : 2048; }
};

#endif // CDIDISC_HPP
