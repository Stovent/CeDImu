#include "CDIDirectory.hpp"
#include "CDIDisc.hpp"
#include "common/filesystem.hpp"
#include "common/utils.hpp"

#include <string>

CDIDirectory::CDIDirectory(uint8_t namesize, std::string dirName, uint32_t lbn, uint16_t parent, uint16_t offset)
    : nameSize(namesize)
    , name(dirName)
    , LBN(lbn)
    , parentDirectory(parent)
    , relOffset(offset)
    , files()
    , subdirectories()
{
}

/** \brief Load the content of the directory.
 *
 * \param  disc A reference to the CDIDisc containing the directory.
 *
 * Reads the directory content from the disc and recursively loads its subdirectories' content.
 */
void CDIDirectory::LoadContent(CDIDisc& disc)
{
    const uint32_t pos = disc.Tell();
    uint8_t c;

    disc.GotoLBN(LBN);
    c = disc.GetByte();
    disc.Seek(c - 1, std::ios::cur); // describe the current directory, so we skip it
    c = disc.GetByte();
    disc.Seek(c - 1, std::ios::cur); // describe the parent directory, so we skip it

    do
    {
        while((disc.Tell() % 2352) < 2072)
        {
            uint8_t namesize, filenumber;
            uint16_t attributes;
            uint32_t lbn, filesize;
            std::string name;

            c = disc.GetByte(); // record length
            if(c == 0)
                break;

            disc.Seek(5, std::ios::cur);
            lbn = disc.GetLong();

            disc.Seek(4, std::ios::cur);
            filesize = disc.GetLong();

            disc.Seek(14, std::ios::cur);

            namesize = disc.GetByte();
            name = disc.GetString(namesize, 0);
            if(isEven(namesize))
                disc.GetByte();

            disc.Seek(4, std::ios::cur); // skip Owner ID
            attributes = disc.GetWord();

            disc.Seek(2, std::ios::cur);
            filenumber = disc.GetByte();
            disc.GetByte(); // last byte is reserved

            if(attributes & 0x8000) // directory
            {
                std::pair<std::map<std::string, CDIDirectory>::iterator, bool> dir = subdirectories.emplace(name, CDIDirectory(namesize, name, lbn, relOffset, 0));
                if(dir.second)
                {
                    dir.first->second.LoadContent(disc);
                }
            }
            else // file
            {
                files.emplace(name, CDIFile(disc, lbn, filesize, namesize, name, attributes, filenumber, relOffset));
            }
        }
        if(!(disc.subheader.submode & cdieof))
            disc.GotoNextSector();
    } while(!(disc.subheader.submode & cdieof)); // in case the directory structure is spreaded over several sectors

    disc.Seek(pos);
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
        filename = filename.substr(pos + 1);

        std::map<std::string, CDIDirectory>::iterator it = subdirectories.find(dir);
        if(it == subdirectories.end())
            return nullptr;

        return it->second.GetFile(filename);
    }
}

/** \brief Clear the directory content.
 *
 * Delete the loaded files and subdirectories of this directory (does not delete them from the disc).
 */
void CDIDirectory::Clear()
{
    files.clear();
    subdirectories.clear();
}

/** \brief Returns the directory structure as a stringstream.
 *
 * \return A stringstream containing the directory structure.
 *
 * The structure is designed as follow:
 * ```
 * Dir: <name>/
 * LBN: <logical block number of the directory>
 *     File: <file name>
 *     Size: <file size>
 *     LBN : <logical block number of the file>
 * ```
 */
std::stringstream CDIDirectory::GetChildrenTree() const
{
    std::stringstream ss;
    std::string dirName = name == "/" ? "" : name;
    ss << "Dir: " << dirName << "/" << std::endl;
    ss << "LBN: " << LBN << std::endl;

    for(const std::pair<std::string, CDIDirectory> dir : subdirectories)
    {
        std::stringstream dirss = dir.second.GetChildrenTree();
        std::string line;
        while(std::getline(dirss, line))
            ss << "\t" << line << std::endl;
    }

    for(const std::pair<std::string, CDIFile> file : files)
    {
        ss << "\tFile: " << file.second.name << std::endl;
        ss << "\tSize: " << file.second.size << std::endl;
        ss << "\tLBN : " << file.second.LBN << std::endl << std::endl;
    }

    return ss;
}

/** \brief Export the audio data from each file in the directory and its subdirectories recursively.
 *
 * \param  basePath The directory where the files will be written.
 */
void CDIDirectory::ExportAudio(std::string basePath) const
{
    if(name != "/")
        basePath += name + "/";

    if(!createDirectories(basePath))
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

/** \brief Export the content of each file in the directory and its subdirectories recursively.
 *
 * \param basePath The directory where the files will be written.
 */
void CDIDirectory::ExportFiles(std::string basePath) const
{
    if(name != "/")
        basePath += name + "/";

    if(!createDirectories(basePath))
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

/** \brief Export the video data from each file in the directory and its subdirectories recursively.
 *
 * \param  basePath The directory where the files will be written.
 */
void CDIDirectory::ExportVideo(std::string basePath) const
{
    if(name != "/")
        basePath += name + "/";

    if(!createDirectories(basePath))
        return;

    for(std::pair<std::string, CDIFile> file : files)
    {
        file.second.ExportVideo(basePath);
    }

    for(std::pair<std::string, CDIDirectory> subdir : subdirectories)
    {
        subdir.second.ExportVideo(basePath);
    }
}

/** \brief Export the raw video data from each file in the directory and its subdirectories recursively.
 *
 * \param  basePath The base directory where the files will be written.
 */
void CDIDirectory::ExportRawVideo(std::string basePath) const
{
    if(name != "/")
        basePath += name + "/";

    if(!createDirectories(basePath))
        return;

    for(std::pair<std::string, CDIFile> file : files)
    {
        file.second.ExportRawVideo(basePath);
    }

    for(std::pair<std::string, CDIDirectory> subdir : subdirectories)
    {
        subdir.second.ExportRawVideo(basePath);
    }
}
