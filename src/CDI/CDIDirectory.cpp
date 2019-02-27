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

void CDIDirectory::Export(CDI& cdi, std::string basePath) const
{
    uint32_t pos = cdi.disk.tellg();
    for(std::pair<std::string, CDIFile> val : files)
    {
        const CDIFile& file = val.second;
        std::ofstream out(basePath + file.name, std::ios::binary);
        int64_t size = file.size;
        char s[2324];
        cdi.disk.seekg(file.LBN + 24);
        while(size > 0)
        {
            uint16_t sectorSize = (cdi.subheader.Submode & cdiform) ? 2324 : 2048;
            uint16_t dtr = (size > sectorSize) ? sectorSize : size; // data to retrieve
            cdi.disk.read(s, dtr);
            out.write(s, dtr);
            size -= dtr;
            cdi.GotoNextSector();
        }
        out.close();
    }

    for(std::pair<std::string, CDIDirectory> value : subDirectories)
    {
        std::string dirPath = basePath + name + "/";
        if(!wxDirExists(dirPath))
            if(!wxMkdir(dirPath))
                return;
        value.second.Export(cdi, dirPath);
    }
    cdi.disk.seekg(pos);
}
