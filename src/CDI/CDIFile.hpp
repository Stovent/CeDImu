#ifndef CDIFILE_HPP
#define CDIFILE_HPP

struct CDIFile;

#include <string>

#include "CDIDisk.hpp"

struct CDIFile
{
    CDIDisk* disk;
    uint8_t nameSize;
    uint8_t fileNumber;
    uint16_t attributes;
    uint16_t parent;
    uint32_t fileLBN;
    uint32_t filesize;
    std::string name;

    CDIFile(CDIDisk* cdidisk);
    CDIFile(CDIDisk* cdidisk, uint32_t lbn, uint32_t filesize, uint8_t namesize, std::string filename, uint16_t attr, uint8_t fileNumber, uint16_t parentRelpos);

    void ExportAudio(std::string directoryPath);
    void ExportFile(std::string directoryPath);
    char* GetFileContent(bool includeModuleHeader = false, uint32_t* size = nullptr);
};

#endif // CDIFILE_HPP
