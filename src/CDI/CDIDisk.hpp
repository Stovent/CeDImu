#ifndef CDIDISK_HPP
#define CDIDISK_HPP

class CDIDisk;

#include <fstream>

typedef struct
{
    uint8_t Minutes;
    uint8_t Seconds;
    uint8_t Sectors;
    uint8_t Mode; // should be 2 for CD-I tracks
} CDIHeader;

typedef struct
{
    uint8_t FileNumber;
    uint8_t ChannelNumber;
    uint8_t Submode;
    uint8_t CodingInformation;
} CDISubheader;

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
};

class CDIDisk
{
    std::ifstream disk;

    void UpdateSectorInfo();

public:
    CDIHeader header;
    CDISubheader subheader;

    bool Open(const std::string& filename);
    bool IsOpen();
    void Close();
    bool Good();
    void Clear();
    int Peek();

    uint32_t Tell();
    void Seek(const uint32_t offset, std::ios::seekdir direction = std::ios::beg);
    bool GotoLBN(const uint32_t lbn, const uint32_t offset = 0);
    bool GotoNextSector(uint8_t submodeMask = 0, const bool maskIncludeCurrentSector = false, uint32_t maxSectorCount = UINT32_MAX, const bool includeAllSectors = false);

    bool GetData(char* dst, uint32_t size, const bool includeEmptySectors = false);
    bool Read(char* dst, uint32_t size);
    uint8_t  GetByte();
    uint16_t GetWord();
    uint32_t GetLong();
    std::string GetString(uint16_t length = 128, const char delim = ' ');

    bool IsEmptySector();
    uint16_t GetSectorDataSize();
};

#endif // CDIDISK_HPP
