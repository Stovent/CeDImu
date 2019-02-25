#include <sstream>

#include <wx/filefn.h>

#include "CDI.hpp"

bool CDI::ExportFiles()
{
    ExportFilesInfo();
    bool bol = true;
    uint32_t pos = disk.tellg();
    std::string currentPath = romPath + "exports";
    if(!wxDirExists(currentPath))
        if(!wxMkdir(currentPath))
            return false;
    currentPath += "/files/";
    if(!wxDirExists(currentPath))
        if(!wxMkdir(currentPath))
            return false;

    for(auto value : rootDirectory.subDirectories)
    {
        const CDIDirectory& dir = value.second;
        std::string path = currentPath;
        if(dir.name != "/")
        {
            path += dir.name + "/";
        }
        if(!wxDirExists(path))
            if(!wxMkdir(path))
                return false;
        dir.Export(*this, path);
    }

    for(std::pair<std::string, CDIFile> val : rootDirectory.files)
    {
        const CDIFile& file = val.second;
        std::ofstream out(currentPath + file.name, std::ios::binary);
        int64_t size = file.size;
        char s[2324];
        if(!GotoLBN(file.LBN))
            { bol = false; break; }
        while(size > 0)
        {
            if(!disk.good())
                { bol = false; break; }
            uint16_t sectorSize = (subheader.Submode & cdiform) ? 2324 : 2048;
            uint16_t dtr = (size > sectorSize) ? sectorSize : size; // data to retrieve
            disk.read(s, dtr);
            out.write(s, dtr);
            size -= dtr;
            GotoNextSector();
        }
        out.close();
    }

    disk.seekg(pos);
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
