#include "CDI.hpp"

#include <sstream>
#include <iomanip>

#include <wx/msgdlg.h>

#include "../utils.hpp"

bool CDI::ExportAudio()
{
    if(!disk.IsOpen())
    {
        wxMessageBox("No ROM loaded, no audio to export");
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

void CDI::ExportAudioInfo()
{
    if(!CreateSubfoldersFromROMDirectory())
        wxMessageBox("Could not create subfolders " + gameFolder);

    std::ofstream out(gameFolder + "audio_info.txt");

    out.close();
}

bool CDI::ExportFiles()
{
    if(!disk.IsOpen())
    {
        wxMessageBox("No ROM loaded, no files to export");
        return false;
    }

    ExportSectorsInfo();
    ExportFilesInfo();

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

void CDI::ExportFilesInfo()
{
    if(!CreateSubfoldersFromROMDirectory())
        wxMessageBox("Could not create subfolders " + gameFolder);

    std::ofstream out(gameFolder + "files_info.txt");

    out << "Dir: " << rootDirectory.dirname << std::endl;
    out << "LBN: " << rootDirectory.dirLBN << std::endl;

    for(std::pair<std::string, CDIDirectory> dir : rootDirectory.subdirectories)
    {
        std::stringstream ss = dir.second.ExportInfo();
        std::string str, tmp;
        while(std::getline(ss, tmp))
            str += "    " + tmp + "\n";
        out << str;
    }

    for(std::pair<std::string, CDIFile> file : rootDirectory.files)
    {
        std::stringstream ss;
        ss << "    file: " << file.second.filename << std::endl;
        ss << "    Size: " << file.second.filesize << std::endl;
        ss << "    LBN : " << file.second.fileLBN << std::endl << std::endl;
        out << ss.str();
    }

    out.close();
}

void CDI::ExportSectorsInfo()
{
    if(!CreateSubfoldersFromROMDirectory())
    {
        wxMessageBox("Could not create subfolders " + gameFolder);
        return;
    }

    const uint32_t pos = disk.Tell();
    uint32_t LBN = 0;
    disk.Seek(0);

    std::ofstream out(gameFolder + "sectors.txt");
    out << "   LBN Min secs sect mode file channel  submode codingInfo" << std::endl;

    while(disk.Good())
    {
        out << std::right << std::setw(6) << std::to_string(LBN++) \
            << std::setw(4) << std::to_string(disk.header.Minutes) \
            << std::setw(5) << std::to_string(disk.header.Seconds) \
            << std::setw(5) << std::to_string(disk.header.Sectors) \
            << std::setw(5) << std::to_string(disk.header.Mode) \
            << std::setw(5) << std::to_string(disk.subheader.FileNumber) \
            << std::setw(8) << std::to_string(disk.subheader.ChannelNumber) \
            << std::setw(9) << toBinString(disk.subheader.Submode, 8) \
            << std::setw(11) << toBinString(disk.subheader.CodingInformation, 8) << std::endl;
        disk.GotoNextSector(); // pass 'cdiany' argument to remove empty sectors
    }
    disk.Clear();

    out.close();
    disk.Seek(pos);
}
