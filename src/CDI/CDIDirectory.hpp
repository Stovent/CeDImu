#ifndef CDIDIRECTORY_HPP
#define CDIDIRECTORY_HPP

struct CDIDirectory;

#include <map>
#include <string>

#include "CDIDisk.hpp"
#include "CDIFile.hpp"

struct CDIDirectory
{
    uint8_t nameSize;
    uint16_t relOffset;
    uint16_t parentDirectory;
    uint32_t dirLBN;
    std::string name;
    std::string path;
    std::map<std::string, CDIFile> files;
    std::map<std::string, CDIDirectory> subDirectories;

    CDIDirectory(uint8_t namesize, std::string dirname, uint32_t lbn, uint16_t parent, uint16_t offset);

    void LoadSubDirectories(CDIDisk& disk);
    void LoadFiles(CDIDisk& disk);
    bool GetFile(std::string filename, CDIFile& cdifile);
    void Clear();

    std::stringstream ExportInfo() const;
    void ExportAudio(std::string basePath) const;
    void ExportFiles(std::string basePath) const;
};

#endif // CDIDIRECTORY_HPP
