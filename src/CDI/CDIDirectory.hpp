#ifndef CDIDIRECTORY_HPP
#define CDIDIRECTORY_HPP

struct CDIDirectory;

#include <map>
#include <string>
#include <fstream>

#include "../utils.hpp"
#include "CDIFile.hpp"

struct CDIDirectory
{
    uint8_t nameSize;
    uint16_t relOffset;
    uint16_t parentDirectory;
    uint32_t LBN;
    std::string name;
    std::string path;
    std::map<std::string, CDIFile> files;
    std::map<std::string, CDIDirectory> subDirectories;

    CDIDirectory(uint8_t namesize, std::string dirname, uint32_t lbn, uint16_t parent, uint16_t offset);
    void LoadSubDirectories(std::ifstream& disk);
    void LoadFiles(std::ifstream& disk);
    std::stringstream ExportInfo() const;

    void ExportFiles(CDI& cdi, std::string basePath) const;
    void ExportAudio(CDI& cdi, std::string basePath) const;
};

#endif // CDIDIRECTORY_HPP
