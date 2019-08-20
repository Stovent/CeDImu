#include "CDIFile.hpp"

#include <cmath>
#include <fstream>

#include "CDI.hpp"

CDIFile::CDIFile()
{
    nameSize = 0;
    name = "";
    size = 0;
    LBN = 0;
    attributes = 0;
    parent = 0;
    fileNumber = 0;
}

CDIFile::CDIFile(uint32_t lbn, uint32_t filesize, uint8_t namesize, std::string filename, uint16_t attr, uint8_t filenumber, uint16_t parentRelpos)
{
    nameSize = namesize;
    name = filename;
    size = filesize;
    LBN = lbn;
    attributes = attr;
    parent = parentRelpos;
    fileNumber = filenumber;
}


void CDIFile::ExportAudio(CDI& cdi, std::string directoryPath)
{
    uint32_t pos = cdi.disk.tellg();
    int32_t sizeLeft = size;
    uint8_t* data = new uint8_t[1000000];

    cdi.GotoLBN(LBN);

    while(sizeLeft > 0)
    {
        if(cdi.IsEmptySector())
        {
            cdi.GotoNextSector();
        }

        uint16_t sectorSize = (cdi.subheader.Submode & cdiform) ? 2324 : 2048;
        sizeLeft -= (sizeLeft > sectorSize) ? sectorSize : sizeLeft;

        if(!(cdi.subheader.Submode & SubmodeBits::cdia))
            continue;

        char SPB[18][16]; // sound parameters bytes
        char SA[18][112]; // sample audio

        for(uint8_t j = 0; j < 18; j++)
        {
            cdi.disk.read(SPB[j], 16);
            cdi.disk.read(SA[j], 112);
        }

        bool emph = (cdi.subheader.CodingInformation & AudioCodingInformation::emphasis) >> 6;
        uint8_t bps = (cdi.subheader.CodingInformation & AudioCodingInformation::bps) >> 4;
        uint8_t sf = (cdi.subheader.CodingInformation & AudioCodingInformation::sf) >> 2;
        uint8_t ms = cdi.subheader.CodingInformation & AudioCodingInformation::ms;

        cdi.GotoNextSector();
    }

    cdi.disk.seekg(pos);
}

void CDIFile::ExportFile(CDI& cdi, std::string directoryPath)
{
    uint32_t pos = cdi.disk.tellg();

    std::ofstream out(directoryPath + name, std::ios::out | std::ios::binary);
    int32_t sizeLeft = size;
    char s[2324] = {0};
    cdi.GotoLBN(LBN);

    while(sizeLeft > 0)
    {
        uint16_t sectorSize = (cdi.subheader.Submode & cdiform) ? 2324 : 2048;
        uint16_t dtr = (sizeLeft > sectorSize) ? sectorSize : sizeLeft; // Data To Retrieve
        cdi.disk.read(s, dtr);
        out.write(s, dtr);
        sizeLeft -= dtr;
        cdi.GotoNextSector();
    }

    out.close();
    cdi.disk.seekg(pos);
}
