#include "CDIDirectory.hpp"

#include <sstream>

#include <wx/filefn.h>

#include "CDI.hpp"
#include "../utils.hpp"

CDIDirectory::CDIDirectory(uint8_t namesize, std::string dirname, uint32_t lbn, uint16_t parent, uint16_t offset)
{
    nameSize = namesize;
    relOffset = offset;
    parentDirectory = parent;
    dirLBN = lbn;
    name = dirname;
}

void CDIDirectory::LoadSubDirectories(CDIDisk& disk)
{
    uint8_t c;
    uint32_t pos = disk.Tell();

    disk.GotoLBN(dirLBN);
    c = disk.GetByte();
    disk.Seek(c-1, std::ios::cur); // describe the current directory, so we skip it
    c = disk.GetByte();
    disk.Seek(c-1, std::ios::cur); // describe the parent directory, so we skip it

    do
    {
        while((disk.Tell() % 2352) < 2072)
        {
            uint8_t namesize = 0, attr = 0;
            uint32_t lbn = 0;
            std::string dirname;

            c = disk.GetByte(); // record length
            if(c == 0)
                break;

            disk.Seek(5, std::ios::cur);
            lbn = disk.GetLong();

            disk.Seek(22, std::ios::cur); // we skip filesize as it is not necessary for a directory

            namesize = disk.GetByte();
            dirname = disk.GetString(namesize);
            if(isEven(namesize))
                disk.GetByte();

            disk.Seek(4, std::ios::cur);
            attr = disk.GetByte(); // I only retrieves the high order byte...
            disk.Seek(5, std::ios::cur);

            if(attr & 0x80) // ...So I only have to compare with 0x80 and not 0x8000
            {
                subDirectories.emplace(dirname, CDIDirectory(namesize, dirname, lbn, relOffset, 0));
                subDirectories.find(dirname)->second.LoadFiles(disk);
                subDirectories.find(dirname)->second.LoadSubDirectories(disk);
            }
        }
        break;
        disk.GotoNextSector();
    } while(!(disk.subheader.Submode & cdieof)); // in case the directory structure is spreaded over several sectors
    disk.Seek(pos);
}

void CDIDirectory::LoadFiles(CDIDisk& disk)
{
    uint8_t c;
    uint32_t pos = disk.Tell();

    disk.GotoLBN(dirLBN);
    c = disk.GetByte();
    disk.Seek(c-1, std::ios::cur); // describe the current directory, so we skip it
    c = disk.GetByte();
    disk.Seek(c-1, std::ios::cur); // describe the parent directory, so we skip it

    do
    {
        while((disk.Tell() % 2352) < 2072)
        {
            uint8_t namesize = 0, filenbr;
            uint16_t attributes = 0;
            uint32_t lbn = 0, filesize = 0;
            std::string filename;

            c = disk.GetByte(); // record length
            if(c == 0)
                break;

            disk.Seek(5, std::ios::cur);
            lbn = disk.GetLong();

            disk.Seek(4, std::ios::cur);
            filesize = disk.GetLong();

            disk.Seek(14, std::ios::cur);

            namesize = disk.GetByte();
            filename = disk.GetString(namesize);
            if(isEven(namesize))
                disk.GetByte();

            disk.Seek(4, std::ios::cur);

            attributes = disk.GetWord();
            disk.Seek(2, std::ios::cur);
            filenbr = disk.GetByte();
            disk.GetByte(); // last byte is reserved

            if(!(attributes & 0x8000))
            {
                files.emplace(filename, CDIFile(&disk, lbn, filesize, namesize, filename, attributes, filenbr, relOffset));
            }
        }
        break;
        disk.GotoNextSector();
    } while(!(disk.subheader.Submode & cdieof)); // in case the directory structure is spreaded over several sectors
    disk.Seek(pos);
}

bool CDIDirectory::GetFile(std::string filename, CDIFile& cdifile)
{
    const size_t pos = filename.find('/');
    if(pos == std::string::npos)
    {
        std::map<std::string, CDIFile>::iterator it = files.find(filename);
        if(it == files.end())
            return false;

        cdifile = it->second;
        return true;
    }
    else
    {
        std::string dir = filename.substr(0, pos);
        filename = filename.substr(pos+1);

        std::map<std::string, CDIDirectory>::iterator it = subDirectories.find(dir);
        if(it == subDirectories.end())
            return false;

        return it->second.GetFile(filename, cdifile);
    }
}

std::stringstream CDIDirectory::ExportInfo() const
{
    std::stringstream ss;
    ss << "Dir: " << name << "/" << std::endl;
    ss << "LBN: " << dirLBN << std::endl;
    for(std::pair<std::string, CDIFile> file : files)
    {
        ss << "    file: " << file.second.name << std::endl;
        ss << "    Size: " << file.second.filesize << std::endl;
        ss << "    LBN : " << file.second.fileLBN << std::endl << std::endl;
    }
    ss << std::endl;
    return ss;
}

void CDIDirectory::ExportAudio(std::string basePath) const
{
    if(name != "/")
        basePath += name + "/";

    if(!wxDirExists(basePath))
        if(!wxMkdir(basePath))
            return;

    for(std::pair<std::string, CDIFile> file : files)
    {
        file.second.ExportAudio(basePath);
    }

    for(std::pair<std::string, CDIDirectory> subdir : subDirectories)
    {
        subdir.second.ExportAudio(basePath);
    }
}

void CDIDirectory::ExportFiles(std::string basePath) const
{
    if(name != "/")
        basePath += name + "/";

    if(!wxDirExists(basePath))
        if(!wxMkdir(basePath))
            return;

    for(std::pair<std::string, CDIFile> file : files)
    {
        file.second.ExportFile(basePath);
    }

    for(std::pair<std::string, CDIDirectory> subdir : subDirectories)
    {
        subdir.second.ExportFiles(basePath);
    }
}

void CDIDirectory::Clear()
{
    files.clear();
    subDirectories.clear();
}
