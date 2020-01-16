#include "CDIFile.hpp"

#include <cmath>
#include <fstream>

#include "CDI.hpp"
#include "../utils.hpp"

CDIFile::CDIFile(CDIDisk* cdidisk)
{
    disk = cdidisk;
    nameSize = 0;
    fileNumber = 0;
    attributes = 0;
    parent = 0;
    fileLBN = 0;
    filesize = 0;
    name = "";
}

CDIFile::CDIFile(CDIDisk* cdidisk, uint32_t lbn, uint32_t size, uint8_t namesize, std::string filename, uint16_t attr, uint8_t filenumber, uint16_t parentRelpos)
{
    disk = cdidisk;
    nameSize = namesize;
    fileNumber = filenumber;
    attributes = attr;
    parent = parentRelpos;
    fileLBN = lbn;
    filesize = size;
    name = filename;
}

void CDIFile::ExportAudio(std::string directoryPath)
{
    uint32_t pos = disk->Tell();
    int32_t sizeLeft = filesize;
    uint8_t* data = new uint8_t[1000000];

    disk->GotoLBN(fileLBN);

    while(sizeLeft > 0)
    {
        if(disk->IsEmptySector())
        {
            disk->GotoNextSector();
        }

        uint16_t sectorSize = (disk->subheader.Submode & cdiform) ? 2324 : 2048;
        sizeLeft -= (sizeLeft > sectorSize) ? sectorSize : sizeLeft;

        if(!(disk->subheader.Submode & SubmodeBits::cdia))
            continue;

        char SPB[18][16]; // sound parameters bytes
        char SA[18][112]; // sample audio

        for(uint8_t j = 0; j < 18; j++)
        {
            disk->Read(SPB[j], 16);
            disk->Read(SA[j], 112);
        }

        bool emph = (disk->subheader.CodingInformation & AudioCodingInformation::emphasis) >> 6;
        uint8_t bps = (disk->subheader.CodingInformation & AudioCodingInformation::bps) >> 4;
        uint8_t sf = (disk->subheader.CodingInformation & AudioCodingInformation::sf) >> 2;
        uint8_t ms = disk->subheader.CodingInformation & AudioCodingInformation::ms;

        disk->GotoNextSector();
    }

    disk->Seek(pos);
}

void CDIFile::ExportFile(std::string directoryPath)
{
    const uint32_t pos = disk->Tell();

    std::ofstream out(directoryPath + name, std::ios::out | std::ios::binary);

    char const * const data = GetFileContent(true);
    if(data)
        out.write(data, filesize);
    delete[] data;

    out.close();
    disk->Seek(pos);
}

static uint8_t getAdditionalHeaderSize(const uint8_t moduleType)
{
    switch(moduleType)
    {
    case 1: // program
        return 24;

    case 11: // user trap library
        return 32;

    case 12: // system module (OS-9 component)
    case 13: // file manager
        return 8;

    case 14: // physical device driver
        return 12;

    default:
        return 0;
    }
}

/**
* Remember to delete the returned pointer (allocated with new[])
**/
char* CDIFile::GetFileContent(bool includeModuleHeader, uint32_t* size)
{
    const uint32_t pos = disk->Tell();
    disk->GotoLBN(fileLBN);

    uint16_t sds = disk->GetSectorDataSize();
    uint32_t position = (filesize > sds) ? sds : filesize;
    uint32_t sizeLeft = filesize;
    if(size)
        *size = filesize;
    char* data = new (std::nothrow) char[filesize];

    if(data == nullptr)
    {
        wxMessageBox("Could not allocate memory to export file " + name);
        return nullptr;
    }

    disk->GetData(data, position);

    if(!includeModuleHeader)
        if(data[0] == (char)0x4A && data[1] == (char)0xFC && disk->subheader.Submode & cdid) // CDI Module
        {
            // Header size will be constant but some of its data are at arbitrary locations inside the module, e.g. the module name
            // so this code only works if the name is right after the header and right before the code
            const int16_t dataSize = position - (48 + getAdditionalHeaderSize(data[0x12])) - (isEven(nameSize) ? nameSize+2 : nameSize+1);
            disk->Seek(-dataSize, std::ios_base::cur);
            disk->Read(data, dataSize);
            position = dataSize;
            if(size)
                *size -= dataSize;
        }

    sizeLeft -= position;
    disk->GotoNextSector();
    if(sizeLeft)
        disk->GetData(&data[position], sizeLeft);

    disk->Seek(pos);
    return data;
}
