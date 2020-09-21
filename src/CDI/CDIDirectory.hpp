#ifndef CDIDIRECTORY_HPP
#define CDIDIRECTORY_HPP

struct CDIDirectory;

#include <map>
#include <string>

class CDIDisk;
#include "CDIFile.hpp"

struct CDIDirectory
{
    uint8_t nameSize;
    uint16_t relOffset;
    uint16_t parentDirectory;
    uint32_t dirLBN;
    std::string dirname;
    std::string path;

    std::map<std::string, CDIFile> files;
    std::map<std::string, CDIDirectory> subdirectories;

    CDIDirectory() = delete;
    CDIDirectory(uint8_t namesize, std::string name, uint32_t lbn, uint16_t parent, uint16_t offset);

    void Clear();
    void LoadContent(CDIDisk& disk);
    CDIFile* GetFile(std::string filename);

    std::stringstream ExportContent() const;
    void ExportAudio(std::string basePath) const;
    void ExportFiles(std::string basePath) const;
    void ExportVideo(std::string basePath) const;
};

#endif // CDIDIRECTORY_HPP
