#include <sstream>

#include <wx/filefn.h>

#include "CDI.hpp"
#include "CDIDirectory.hpp"

CDIDirectory::CDIDirectory(uint8_t namesize, std::string dirname, uint32_t lbn, uint16_t parent, uint16_t offset)
{
    relOffset = offset;
    LBN = lbn;
    parentDirectory = parent;
    nameSize = namesize;
    name = dirname;
}

void CDIDirectory::LoadSubDirectories(std::ifstream& disk)
{
    char c;
    uint32_t pos = disk.tellg();

    disk.seekg(LBN * 2352 + 24);
    disk.get(c);
    disk.seekg(c-1, std::ios::cur);
    disk.get(c);
    disk.seekg(c-1, std::ios::cur);

    while((disk.tellg() % 2352) < 2072)
    {
        uint8_t namesize = 0, attr = 0;
        uint32_t lbn = 0;
        std::string dirname;

        disk.seekg(6, std::ios::cur);
        disk.get(c); lbn |= (uint8_t)c << 24;
        disk.get(c); lbn |= (uint8_t)c << 16;
        disk.get(c); lbn |= (uint8_t)c << 8;
        disk.get(c); lbn |= (uint8_t)c;

        disk.seekg(22, std::ios::cur);
        disk.get(c); namesize = c;
        if(namesize == 0)
            break;
        for(int i = 0; i < namesize; i++)
        {
            disk.get(c);
            dirname += (c == 0) ? '/' : c;
        }
        if(isEven(namesize))
            disk.get(c);

        disk.seekg(4, std::ios::cur);
        disk.get(c); attr |= (uint8_t)c; // I only retrieves the high order byte...
        disk.seekg(5, std::ios::cur);

        if(attr & 0x80) // ...So I only have to compare with 0x80 and not 0x8000
        {
            subDirectories.emplace(dirname, CDIDirectory(namesize, dirname, lbn, relOffset, 0));
            subDirectories.find(dirname)->second.LoadFiles(disk);
            subDirectories.find(dirname)->second.LoadSubDirectories(disk);
        }
    }
    disk.seekg(pos);
}

void CDIDirectory::LoadFiles(std::ifstream& disk)
{
    char c;
    uint32_t pos = disk.tellg();

    disk.seekg(LBN * 2352 + 24);
    disk.get(c);
    disk.seekg(c-1, std::ios::cur);
    disk.get(c);
    disk.seekg(c-1, std::ios::cur);

    while((disk.tellg() % 2352) < 2072)
    {
        uint8_t namesize = 0, filenbr;
        uint16_t attr = 0;
        uint32_t lbn = 0, filesize = 0;
        std::string filename;

        disk.seekg(6, std::ios::cur);
        disk.get(c); lbn |= (uint8_t)c << 24;
        disk.get(c); lbn |= (uint8_t)c << 16;
        disk.get(c); lbn |= (uint8_t)c << 8;
        disk.get(c); lbn |= (uint8_t)c;

        disk.seekg(4, std::ios::cur);
        disk.get(c); filesize |= (uint8_t)c << 24;
        disk.get(c); filesize |= (uint8_t)c << 16;
        disk.get(c); filesize |= (uint8_t)c << 8;
        disk.get(c); filesize |= (uint8_t)c;

        disk.seekg(14, std::ios::cur);
        disk.get(c); namesize = (uint8_t)c;
        if(namesize == 0)
            break;
        for(int i = 0; i < namesize; i++)
        {
            disk.get(c);
            filename += (c == 0) ? '/' : c;
        }
        if(isEven(namesize))
            disk.get(c);

        disk.seekg(4, std::ios::cur);
        disk.get(c); attr |= (uint8_t)c << 8;
        disk.get(c); attr |= (uint8_t)c;
        disk.seekg(2, std::ios::cur);
        disk.get(c); filenbr = (uint8_t)c; disk.get(c);

        if(!(attr & 0x8000))
            files.emplace(filename, CDIFile(lbn, filesize, namesize, filename, attr, filenbr, relOffset));
    }
    disk.seekg(pos);
}

std::stringstream CDIDirectory::ExportInfo() const
{
    std::stringstream ss;
    ss << "Dir: " << name << "/" << std::endl;
    ss << "LBN: " << LBN << std::endl;
    for(std::pair<std::string, CDIFile> file : files)
    {
        ss << "    file: " << file.second.name << std::endl;
        ss << "    Size: " << file.second.size << std::endl;
        ss << "    LBN : " << file.second.LBN << std::endl << std::endl;
    }
    ss << std::endl;
    return ss;
}

void CDIDirectory::ExportAudio(CDI& cdi, std::string basePath) const
{
    if(name != "/")
        basePath += name + "/";

    if(!wxDirExists(basePath))
        if(!wxMkdir(basePath))
            return;

    for(std::pair<std::string, CDIFile> file : files)
    {
        file.second.ExportAudio(cdi, basePath);
    }

    for(std::pair<std::string, CDIDirectory> dir : subDirectories)
    {
        dir.second.ExportAudio(cdi, basePath);
    }
}

void CDIDirectory::ExportFiles(CDI& cdi, std::string basePath) const
{
    if(name != "/")
        basePath += name + "/";

    if(!wxDirExists(basePath))
        if(!wxMkdir(basePath))
            return;

    for(std::pair<std::string, CDIFile> file : files)
    {
        file.second.ExportFile(cdi, basePath);
    }

    for(std::pair<std::string, CDIDirectory> value : subDirectories)
    {
        value.second.ExportFiles(cdi, basePath);
    }
}
