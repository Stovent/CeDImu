#ifndef CDI_CDIDIRECTORY_HPP
#define CDI_CDIDIRECTORY_HPP

class CDIDisc;
#include "CDIFile.hpp"

#include <map>
#include <string>

class CDIDirectory
{
public:
    const uint8_t nameSize;
    const std::string name;
    uint32_t LBN;
    const uint16_t parentDirectory;
    const uint16_t relOffset;

    CDIDirectory() = delete;
    CDIDirectory(uint8_t namesize, std::string dirName, uint32_t lbn, uint16_t parent, uint16_t offset);

    void Clear();
    void LoadContent(CDIDisc& disc);
    CDIFile* GetFile(std::string filename);

    std::stringstream GetChildrenTree() const;
    void ExportAudio(std::string basePath) const;
    void ExportFiles(std::string basePath) const;
    void ExportVideo(std::string basePath) const;
    void ExportRawVideo(std::string basePath) const;

private:
    std::map<std::string, CDIFile> files;
    std::map<std::string, CDIDirectory> subdirectories;
};

#endif // CDI_CDIDIRECTORY_HPP
