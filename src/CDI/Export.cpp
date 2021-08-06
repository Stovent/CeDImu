#include "CDIDisc.hpp"
#include "common/utils.hpp"
#include "common/filesystem.hpp"

#include <wx/msgdlg.h>

#include <iomanip>
#include <sstream>

/** \brief Create subdirectories inside the game directory.
 *
 * \param  path The directories to create, separated by '/'.
 * \return false if a directory could not be created, true otherwise.
 *
 * The game directory is the directory where the disc is located +
 * the game name inside the disc.
 * An empty string only creates the game directory only (romPath + gameName).
 * Example: if the game is Alien Gate, and the disc is in C:/ROMs/
 * then sending path = "files/CMDS/" will create C:/ROMs/Alien Gate/files/CMDS/
 */
bool CDIDisc::CreateSubfoldersFromROMDirectory(std::string path)
{
    return createDirectories(gameFolder + path);
}

/** \brief Exports the audio data in the disc.
 *
 * \return false if no disc has been opened or if it could not create subfolders, true otherwise.
 */
bool CDIDisc::ExportAudio()
{
    if(!IsOpen())
    {
        wxMessageBox("No disc loaded, no audio to export");
        return false;
    }

    std::string currentPath = "audio/";
    if(!CreateSubfoldersFromROMDirectory(currentPath))
    {
        wxMessageBox("Could not create subfolders " + currentPath);
        return false;
    }

    currentPath = gameFolder + currentPath;

    rootDirectory.ExportAudio(currentPath);

    return true;
}

/** \brief Exports the files contained in the disc.
 *
 * \return false if no disc has been opened or if it could not create subfolders, true otherwise.
 */
bool CDIDisc::ExportFiles()
{
    if(!IsOpen())
    {
        wxMessageBox("No disc loaded, no files to export");
        return false;
    }

    ExportSectorsInfo();
    ExportFileSystem();

    std::string currentPath = "files/";
    if(!CreateSubfoldersFromROMDirectory(currentPath))
    {
        wxMessageBox("Could not create subfolders " + gameFolder);
        return false;
    }

    currentPath = gameFolder + currentPath;

    rootDirectory.ExportFiles(currentPath);

    return true;
}

/** \brief Export the strucure of the disc's file system.
 */
void CDIDisc::ExportFileSystem()
{
    if(!CreateSubfoldersFromROMDirectory())
        wxMessageBox("Could not create subfolders " + gameFolder);

    std::ofstream out(gameFolder + "files_info.txt");

    out << rootDirectory.GetChildrenTree().str();

    out.close();
}

/** \brief Exports the video data in the disc.
 *
 * \return false if no disc has been opened or if it could not create subfolders, true otherwise.
 */
bool CDIDisc::ExportVideo()
{
    if(!IsOpen())
    {
        wxMessageBox("No disc loaded, no video to export");
        return false;
    }

    std::string currentPath = "video/";
    if(!CreateSubfoldersFromROMDirectory(currentPath))
    {
        wxMessageBox("Could not create subfolders " + currentPath);
        return false;
    }

    currentPath = gameFolder + currentPath;

    rootDirectory.ExportVideo(currentPath);

    return true;
}

/** \brief Exports the raw video data from the disc.
 *
 * \return false if no disc has been opened or if it could not create subfolders, true otherwise.
 */
bool CDIDisc::ExportRawVideo()
{
    if(!IsOpen())
        return false;

    std::string currentPath = "rawvideo/";
    if(!CreateSubfoldersFromROMDirectory(currentPath))
        return false;

    currentPath = gameFolder + currentPath;

    rootDirectory.ExportRawVideo(currentPath);

    return true;
}

/** \brief Exports the structure of the sectors in the disc.
 */
void CDIDisc::ExportSectorsInfo()
{
    if(!CreateSubfoldersFromROMDirectory())
    {
        wxMessageBox("Could not create subfolders " + gameFolder);
        return;
    }

    const uint32_t pos = Tell();
    uint32_t LBN = 0;
    Seek(0);

    std::ofstream out(gameFolder + "sectors.txt");
    out << "   LBN Min secs sect mode file channel  submode codingInfo" << std::endl;

    while(Good())
    {
        out << std::right << std::setw(6) << std::to_string(LBN++) \
            << std::setw(4) << std::to_string(header.minute) \
            << std::setw(5) << std::to_string(header.second) \
            << std::setw(5) << std::to_string(header.sector) \
            << std::setw(5) << std::to_string(header.mode) \
            << std::setw(5) << std::to_string(subheader.fileNumber) \
            << std::setw(8) << std::to_string(subheader.channelNumber) \
            << std::setw(9) << toBinString(subheader.submode, 8) \
            << std::setw(11) << toBinString(subheader.codingInformation, 8) << std::endl;
        GotoNextSector();
    }
    disc.clear();

    out.close();
    Seek(pos);
}
