#ifndef CDIFILE_HPP
#define CDIFILE_HPP

class CDIDisc;

#include <string>

class CDIFile
{
    CDIDisc& disc;

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

    void ExportAudio(const std::string& directoryPath);
    void ExportFile(const std::string& directoryPath);
    void ExportVideo(const std::string& directoryPath);
    uint8_t* GetContent(uint32_t& size);
};

#endif // CDIFILE_HPP
