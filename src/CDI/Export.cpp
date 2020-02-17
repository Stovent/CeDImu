#include <sstream>

#include <wx/filefn.h>

#include "CDI.hpp"
#include "../utils.hpp"

bool CDI::ExportAudio()
{
    if(!disk.IsOpen())
    {
        wxMessageBox("No ROM loaded, no audio to export");
        return false;
    }

    cedimu->mainFrame->SetStatusText("Exporting audio...");

    std::string currentPath = "audio/";
    if(!CreateSubfoldersFromROMDirectory(currentPath))
    {
        wxMessageBox("Could not create subfolders " + currentPath);
        return false;
    }

    currentPath = gameFolder + currentPath;

    rootDirectory.ExportAudio(currentPath);

    cedimu->mainFrame->SetStatusText("Audio exported to " + currentPath);
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

    cedimu->mainFrame->SetStatusText("Exporting files...");
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

    cedimu->mainFrame->SetStatusText("Files exported to " + currentPath);
    return true;
}

void CDI::ExportFilesInfo()
{
    if(!CreateSubfoldersFromROMDirectory())
        wxMessageBox("Could not create subfolders " + gameFolder);

    std::ofstream out(gameFolder + "files_info.txt");

    out << "Dir: " << rootDirectory.name << std::endl;
    out << "LBN: " << rootDirectory.dirLBN << std::endl;

    for(std::pair<std::string, CDIDirectory> dir : rootDirectory.subDirectories)
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
        ss << "    file: " << file.second.name << std::endl;
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

    std::ofstream out(gameFolder + "sectors.txt");

    const uint32_t pos = disk.Tell();
    disk.Seek(0);

    out << "LBN  Min secs sect mode file channel submode  codingInfo" << std::endl;
    uint32_t LBN = 0;
    while(disk.Good())
    {
        out << std::to_string(LBN++) << "   " << std::to_string(disk.header.Minutes) << "   " << std::to_string(disk.header.Seconds) << "   " << std::to_string(disk.header.Sectors) << "   " << std::to_string(disk.header.Mode) << "    ";
        out << std::to_string(disk.subheader.FileNumber) << "    " << std::to_string(disk.subheader.ChannelNumber) << " " << toBinString(disk.subheader.Submode, 8) << " " << toBinString(disk.subheader.CodingInformation, 8) << std::endl;
        disk.GotoNextSector(0, false, UINT32_MAX, false);
    }
    disk.Clear();

    out.close();
    disk.Seek(pos);
}
