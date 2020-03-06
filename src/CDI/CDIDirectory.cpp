#include "CDIDirectory.hpp"

#include <wx/msgdlg.h>

#include "../utils.hpp"

CDIDirectory::CDIDirectory(uint8_t namesize, std::string name, uint32_t lbn, uint16_t parent, uint16_t offset)
{
    nameSize = namesize;
    relOffset = offset;
    parentDirectory = parent;
    dirLBN = lbn;
    dirname = name;
}

/**
* Reads the directory content from the disk and reursively
* loads its subfolders' content.
**/
void CDIDirectory::LoadContent(CDIDisk& disk)
{
    const uint32_t pos = disk.Tell();
    uint8_t c;

    disk.GotoLBN(dirLBN);
    c = disk.GetByte();
    disk.Seek(c-1, std::ios::cur); // describe the current directory, so we skip it
    c = disk.GetByte();
    disk.Seek(c-1, std::ios::cur); // describe the parent directory, so we skip it

    do
    {
        while((disk.Tell() % 2352) < 2072)
        {
            uint8_t namesize, filenumber;
            uint16_t attributes;
            uint32_t lbn, filesize;
            std::string name;

            c = disk.GetByte(); // record length
            if(c == 0)
                break;

            disk.Seek(5, std::ios::cur);
            lbn = disk.GetLong();

            disk.Seek(4, std::ios::cur);
            filesize = disk.GetLong();

            disk.Seek(14, std::ios::cur);

            namesize = disk.GetByte();
            name = disk.GetString(namesize, 0);
            if(isEven(namesize))
                disk.GetByte();

            disk.Seek(4, std::ios::cur); // skip Owner ID
            attributes = disk.GetWord();

            disk.Seek(2, std::ios::cur);
            filenumber = disk.GetByte();
            disk.GetByte(); // last byte is reserved

            if(attributes & 0x8000) // directory
            {
                std::pair<std::map<std::string, CDIDirectory>::iterator, bool> dir = subdirectories.emplace(name, CDIDirectory(namesize, name, lbn, relOffset, 0));
                if(dir.second)
                {
                    dir.first->second.LoadContent(disk);
                }
                else
                    wxMessageBox(name + " already exists in the current directory " + dirname);
            }
            else // file
            {
                files.emplace(name, CDIFile(disk, lbn, filesize, namesize, name, attributes, filenumber, relOffset));
            }
        }
        if(!(disk.subheader.Submode & cdieof))
            disk.GotoNextSector();
    } while(!(disk.subheader.Submode & cdieof)); // in case the directory structure is spreaded over several sectors
    disk.Seek(pos);
}

/**
* Returns a pointer to the file named {filename}, or nullptr is not found
**/
CDIFile* CDIDirectory::GetFile(std::string filename)
{
    const size_t pos = filename.find('/');
    if(pos == std::string::npos)
    {
        std::map<std::string, CDIFile>::iterator it = files.find(filename);
        if(it == files.end())
            return nullptr;

        return &it->second;
    }
    else
    {
        std::string dir = filename.substr(0, pos);
        filename = filename.substr(pos+1);

        std::map<std::string, CDIDirectory>::iterator it = subdirectories.find(dir);
        if(it == subdirectories.end())
            return nullptr;

        return it->second.GetFile(filename);
    }
}

/**
* Clear the directory content.
**/
void CDIDirectory::Clear()
{
    files.clear();
    subdirectories.clear();
}

/**
* Returns the directory organization.
**/
std::stringstream CDIDirectory::ExportInfo() const
{
    std::stringstream ss;
    ss << "Dir: " << dirname << "/" << std::endl;
    ss << "LBN: " << dirLBN << std::endl;
    for(std::pair<std::string, CDIFile> file : files)
    {
        ss << "    file: " << file.second.filename << std::endl;
        ss << "    Size: " << file.second.filesize << std::endl;
        ss << "    LBN : " << file.second.fileLBN << std::endl << std::endl;
    }
    ss << std::endl;
    return ss;
}

/**
* Exports the files audio data, and recursively exports its subdirectories'
* files audio data.
**/
void CDIDirectory::ExportAudio(std::string basePath) const
{
    if(dirname != "/")
        basePath += dirname + "/";

    if(!wxDirExists(basePath))
        if(!wxMkdir(basePath))
            return;

    for(std::pair<std::string, CDIFile> file : files)
    {
        file.second.ExportAudio(basePath);
    }

    for(std::pair<std::string, CDIDirectory> subdir : subdirectories)
    {
        subdir.second.ExportAudio(basePath);
    }
}

/**
* Exports the files content, and recursively exports its subdirectories'
* files content.
**/
void CDIDirectory::ExportFiles(std::string basePath) const
{
    if(dirname != "/")
        basePath += dirname + "/";

    if(!wxDirExists(basePath))
        if(!wxMkdir(basePath))
            return;

    for(std::pair<std::string, CDIFile> file : files)
    {
        file.second.ExportFile(basePath);
    }

    for(std::pair<std::string, CDIDirectory> subdir : subdirectories)
    {
        subdir.second.ExportFiles(basePath);
    }
}
