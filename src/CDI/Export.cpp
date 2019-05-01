#include <sstream>

#include <wx/filefn.h>

#include "CDI.hpp"

bool CDI::ExportAudio()
{
    ExportAudioInfo();
    bool bol = true;

    std::string currentPath = romPath + "exports/";
    if(!wxDirExists(currentPath))
        if(!wxMkdir(currentPath))
            return false;

    currentPath += "audio/";
    if(!wxDirExists(currentPath))
        if(!wxMkdir(currentPath))
            return false;

    rootDirectory.ExportAudio(*this, currentPath);

    return bol;
}

void CDI::ExportAudioInfo()
{
    if(!wxDirExists(romPath + "exports"))
        if(!wxMkdir(romPath + "exports"))
            return;

    std::ofstream out(romPath + "exports/audio_info.txt");
    out.close();
}

bool CDI::ExportFiles()
{
    ExportSectorsInfo();
    ExportFilesInfo();
    bool bol = true;

    std::string currentPath = romPath + "exports/";
    if(!wxDirExists(currentPath))
        if(!wxMkdir(currentPath))
            return false;

    currentPath += "files/";
    if(!wxDirExists(currentPath))
        if(!wxMkdir(currentPath))
            return false;

    rootDirectory.ExportFiles(*this, currentPath);

    return bol;
}

void CDI::ExportFilesInfo()
{
    if(!wxDirExists(romPath + "exports"))
        if(!wxMkdir(romPath + "exports"))
            return;

    std::ofstream out(romPath + "exports/files_info.txt");
    out << "Dir: " << rootDirectory.name << std::endl;
    out << "LBN: " << rootDirectory.LBN << std::endl;

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
        ss << "    Size: " << file.second.size << std::endl;
        ss << "    LBN : " << file.second.LBN << std::endl << std::endl;
        out << ss.str();
    }

    out.close();
}

void CDI::ExportSectorsInfo()
{
    if(!wxDirExists(romPath + "exports"))
        if(!wxMkdir(romPath + "exports"))
            return;

    uint32_t pos = disk.tellg();
    disk.seekg(0);
    std::ofstream out(romPath + "exports/sectors.txt");

    out << "LBN  Min secs sect mode file channel submode  codingInfo" << std::endl;
    uint32_t LBN = 0;
    while(disk.good())
    {
        out << std::to_string(LBN++) << "   " << std::to_string(header.Minutes) << "   " << std::to_string(header.Seconds) << "   " << std::to_string(header.Sectors) << "   " << std::to_string(header.Mode) << "    ";
        out << std::to_string(subheader.FileNumber) << "    " << std::to_string(subheader.ChannelNumber) << " " << toBinString(subheader.Submode, 8) << " " << toBinString(subheader.CodingInformation, 8) << std::endl;
        GotoNextSector();
        if(!disk.good()) // end of disk reached
        {
            disk.clear();
            disk.seekg(0);
            break;
        }
    }

    out.close();
    disk.seekg(pos);
}
