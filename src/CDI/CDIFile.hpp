#ifndef CDI_CDIFILE_HPP
#define CDI_CDIFILE_HPP

class CDIDisc;
struct CDISector;

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

class CDIFile
{
public:
    const uint32_t LBN;
    const uint32_t size;
    const uint8_t nameSize;
    const std::string name;
    const uint16_t attributes;
    const uint8_t number;
    const uint16_t parent;

    CDIFile() = delete;
    CDIFile(CDIDisc& cdidisc, uint32_t lbn, uint32_t filesize, uint8_t namesize, std::string filename, uint16_t attr, uint8_t filenumber, uint16_t parentRelpos);

    void ExportAudio(const std::string& directoryPath) const;
    void ExportFile(const std::string& directoryPath) const;
    void ExportRawVideo(const std::string& directoryPath) const;
    std::vector<uint8_t> GetContent() const;

    void ForEachSector(std::function<void(const CDISector&)> f) const;

private:
    CDIDisc& disc;
};

#endif // CDI_CDIFILE_HPP
