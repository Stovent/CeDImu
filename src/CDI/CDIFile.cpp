#include "CDIFile.hpp"

#include <fstream>
#include <iterator>

#include "CDI.hpp"
#include "../utils.hpp"

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

    uint32_t size;
    char const * const data = GetFileContent(&size);
    if(data && size)
        out.write(data, size);
    delete[] data;

    out.close();
    disk->Seek(pos);
}

/**
* The parameter is a pointer that, if not null, will contain
* the size of the file content.
* Remember to delete the returned pointer (allocated with new[])
**/
char* CDIFile::GetFileContent(uint32_t* size)
{
    const uint32_t pos = disk->Tell();

    uint32_t readSize = (double)filesize / 2048.0 * 2324.0;
    char* data = new (std::nothrow) char[readSize];
    if(data == nullptr)
    {
        wxMessageBox("Could not allocate memory to export file " + name);
        return nullptr;
    }

    readSize = filesize;
    disk->GotoLBN(fileLBN);
    disk->GetData(data, readSize, true);
    if(size)
        *size = readSize;

    disk->Seek(pos);
    return data;
}
