#include "CDIDirectory.hpp"
#include "CDIDisk.hpp"
#include "../utils.hpp"

#include <wx/msgdlg.h>

#ifdef USE_STD_FILESYSTEM
#include <filesystem>
#else
#include <wx/filefn.h>
#endif // USE_STD_FILESYSTEM

CDIDirectory::CDIDirectory(uint8_t namesize, std::string name, uint32_t lbn, uint16_t parent, uint16_t offset)
{
    nameSize = namesize;
    relOffset = offset;
    parentDirectory = parent;
    dirLBN = lbn;
    dirname = name;
}

/** \brief Load the content of the directory.
 *
 * \param  disk A reference to the CDIDisk containing the directory.
 *
 * Reads the directory content from the disk and recursively loads its subdirectories' content.
 */
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

/** \brief Get file from its path in the directory.
 *
 * \param  filename The path from the current directory to the file.
 * \return A pointer to the file, or nullptr is not found.
 *
 * The path must not start with a '/'.
 * e.g. "CMDS/cdi_gate" for file "cdi_gate" in the "CMDS" folder inside this directory.
 */
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

/** \brief Clear the directory content.
 *
 * Delete the loaded files and subdirectories of this directory (does not delete them from the disk).
 */
void CDIDirectory::Clear()
{
    files.clear();
    subdirectories.clear();
}

/** \brief Returns the directory structure as a stringstream.
 *
 * \return A stringstream containing the directory structure
 *
 * The structure is designed as follow:
 * Dir: <dirname>/
 * LBN: <logical block number of the directory>
 * Then for each file inside the directory:
 *     File: <file name>
 *     Size: <file size>
 *     LBN : <logical block number of the file>
 */
std::stringstream CDIDirectory::ExportContent() const
{
    std::stringstream ss;
    std::string dirName = dirname == '/' ? "" : dirname;
    ss << "Dir: " << dirName << "/" << std::endl;
    ss << "LBN: " << dirLBN << std::endl;

    for(std::pair<std::string, CDIDirectory> dir : subdirectories)
    {
        std::stringstream dirss = dir.second.ExportContent();
        std::string line;
        while(std::getline(dirss, line))
            ss << "\t" << line << std::endl;
    }

    for(std::pair<std::string, CDIFile> file : files)
    {
        ss << "\tFile: " << file.second.filename << std::endl;
        ss << "\tSize: " << file.second.filesize << std::endl;
        ss << "\tLBN : " << file.second.fileLBN << std::endl << std::endl;
    }
    return ss;
}

/** \brief Export the audio data from each file in the directory and its subdirectories recursively.
 *
 * \param  basePath The directory where the files will be written.
 */
void CDIDirectory::ExportAudio(std::string basePath) const
{
    if(dirname != "/")
        basePath += dirname + "/";

#ifdef USE_STD_FILESYSTEM
    if(!std::filesystem::create_directory(basePath))
        return;
#else
    if(!wxDirExists(basePath))
        if(!wxMkdir(basePath))
            return;
#endif // USE_STD_FILESYSTEM

    for(std::pair<std::string, CDIFile> file : files)
    {
        file.second.ExportAudio(basePath);
    }

    for(std::pair<std::string, CDIDirectory> subdir : subdirectories)
    {
        subdir.second.ExportAudio(basePath);
    }
}

/** \brief Export the content of each file in the directory and its subdirectories recursively.
 *
 * \param basePath The directory where the files will be written.
 */
void CDIDirectory::ExportFiles(std::string basePath) const
{
    if(dirname != "/")
        basePath += dirname + "/";

#ifdef USE_STD_FILESYSTEM
    if(!std::filesystem::create_directory(basePath))
        return;
#else
    if(!wxDirExists(basePath))
        if(!wxMkdir(basePath))
            return;
#endif // USE_STD_FILESYSTEM

    for(std::pair<std::string, CDIFile> file : files)
    {
        file.second.ExportFile(basePath);
    }

    for(std::pair<std::string, CDIDirectory> subdir : subdirectories)
    {
        subdir.second.ExportFiles(basePath);
    }
}

/** \brief Export the video data from each file in the directory and its subdirectories recursively.
 *
 * \param  basePath The directory where the files will be written.
 */
void CDIDirectory::ExportVideo(std::string basePath) const
{
    if(dirname != "/")
        basePath += dirname + "/";

#ifdef USE_STD_FILESYSTEM
    if(!std::filesystem::create_directory(basePath))
        return;
#else
    if(!wxDirExists(basePath))
        if(!wxMkdir(basePath))
            return;
#endif // USE_STD_FILESYSTEM

    for(std::pair<std::string, CDIFile> file : files)
    {
        file.second.ExportVideo(basePath);
    }

    for(std::pair<std::string, CDIDirectory> subdir : subdirectories)
    {
        subdir.second.ExportVideo(basePath);
    }
}
