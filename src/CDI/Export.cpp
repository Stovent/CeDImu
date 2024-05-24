#include "CDIDisc.hpp"
#include "common/utils.hpp"

#include <filesystem>
#include <iomanip>
#include <sstream>

/** \brief Exports the audio data in the disc.
 * \param path The directory where to write the data.
 * \return true if the disc was opened, false if no disc was opened.
 * \throw std::filesystem::filesystem_error if it cannot create directories.
 */
bool CDIDisc::ExportAudio(const std::string& path)
{
    if(!IsOpen())
        return false;

    std::string currentPath = path + "/" + m_gameName + "/audio/";

    m_rootDirectory.ExportAudio(currentPath);

    return true;
}

/** \brief Exports the files contained in the disc.
 * \param path The directory where to write the data.
 * \return true if the disc was opened, false if no disc was opened.
 * \throw std::filesystem::filesystem_error if it cannot create directories.
 */
bool CDIDisc::ExportFiles(const std::string& path)
{
    if(!IsOpen())
        return false;

    ExportSectorsInfo(path);
    ExportFileSystem(path);

    std::string currentPath = path + "/" + m_gameName + "/files/";

    m_rootDirectory.ExportFiles(currentPath);

    return true;
}

/** \brief Export the strucure of the disc's file system.
 * \param path The directory where to write the data.
 * \return true if the disc was opened, false if no disc was opened.
 * \throw std::filesystem::filesystem_error if it cannot create directories.
 */
bool CDIDisc::ExportFileSystem(const std::string& path)
{
    if(!IsOpen())
        return false;

    std::string dir = path + "/" + m_gameName + "/";
    std::filesystem::create_directories(dir);

    std::ofstream out(dir + "files_info.txt");

    out << m_rootDirectory.GetChildrenTree().str();

    out.close();

    return true;
}

/** \brief Exports the raw video data from the disc.
 * \param path The directory where to write the data.
 * \return false if no disc has been opened or if it could not create subfolders, true otherwise.
 * \throw std::filesystem::filesystem_error if it cannot create directories.
 */
bool CDIDisc::ExportRawVideo(const std::string& path)
{
    if(!IsOpen())
        return false;

    std::string currentPath = path + "/" + m_gameName + "/rawvideo/";

    m_rootDirectory.ExportRawVideo(currentPath);

    return true;
}

/** \brief Exports the structure of the sectors in the disc.
 * \param path The directory where to write the data.
 * \return true if the disc was opened, false if no disc was opened.
 * \throw std::filesystem::filesystem_error if it cannot create directories.
 */
bool CDIDisc::ExportSectorsInfo(const std::string& path)
{
    if(!IsOpen())
        return false;

    std::string dir = path + "/" + m_gameName + "/";
    std::filesystem::create_directories(dir);

    std::ofstream out(dir + "sectors.txt");
    out << "   LBN Min secs sect mode file channel  submode codingInfo" << std::endl;

    uint32_t LBN = 0;
    ForEachSector([&] (const CDISector& sector) {
        out << std::right << std::setw(6) << std::to_string(LBN++)
            << std::setw(4) << std::to_string(sector.header.minute)
            << std::setw(5) << std::to_string(sector.header.second)
            << std::setw(5) << std::to_string(sector.header.sector)
            << std::setw(5) << std::to_string(sector.header.mode)
            << std::setw(5) << std::to_string(sector.subheader.fileNumber)
            << std::setw(8) << std::to_string(sector.subheader.channelNumber)
            << std::setw(9) << toBinString(sector.subheader.submode, 8)
            << std::setw(11) << toBinString(sector.subheader.codingInformation, 8) << std::endl;
    });

    out.close();

    return true;
}
