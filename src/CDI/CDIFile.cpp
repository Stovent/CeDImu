#include "CDIFile.hpp"

#include <cmath>
#include <fstream>
#include <iterator>
#include <vector>

#include <wx/msgdlg.h>

CDIFile::CDIFile(CDIDisk& cdidisk, uint32_t lbn, uint32_t size, uint8_t namesize, std::string name, uint16_t attr, uint8_t filenumber, uint16_t parentRelpos) : disk(cdidisk)
{
    nameSize = namesize;
    fileNumber = filenumber;
    attributes = attr;
    parent = parentRelpos;
    fileLBN = lbn;
    filesize = size;
    filename = name;
}

/**
* Converts and writes the audio data from the disk to 16-bit PCM on a
* file in the directory {directoryPath}, each channel is exported individualy.
**/
void CDIFile::ExportAudio(std::string directoryPath)
{
    uint32_t pos = disk.Tell();
    float k0[4] = {0.0, 0.9375, 1.796875, 1.53125};
    float k1[4] = {0.0, 0.0, -0.8125, -0.859375};

    for(int channel = 0; channel < 16; channel++)
    {
        WAVHeader wavHeader;
        std::vector<uint16_t> left;
        std::vector<uint16_t> right;

        disk.GotoLBN(fileLBN);

        int32_t sizeLeft = filesize;
        while(sizeLeft > 0)
        {
            sizeLeft -= (sizeLeft < 2048) ? sizeLeft : 2048;

            if(!(disk.subheader.Submode & cdia) || disk.subheader.ChannelNumber != channel)
            {
                disk.GotoNextSector();
                continue;
            }

//            bool emph = disk.subheader.CodingInformation & AudioCodingInformation::emphasis;
            bool bps = disk.subheader.CodingInformation & AudioCodingInformation::bps;
            bool fs = disk.subheader.CodingInformation & AudioCodingInformation::sf;
            bool ms = disk.subheader.CodingInformation & AudioCodingInformation::ms;

            if(bps) // Level A (8 bits per sample)
            {
                for(uint8_t sg = 0; sg < 18; sg++)
                {
                    uint8_t s[4];
                    uint8_t range[4];
                    uint8_t filter[4];
                    uint8_t SD[4][28]; // sound data

                    disk.Read((char*)s, 4);
                    disk.Seek(12, std::ios::cur);
                    for(uint8_t i = 0; i < 4; i++)
                    {
                        range[i] = s[i] & 0x0F;
                        filter[i] = s[i] >> 4;
                    }

                    for(uint8_t ss = 0; ss < 28; ss += 2) // sound sample
                    {
                        for(uint8_t su = 0; su < 4; su++) // sound unit
                        {
                            SD[su][ss] = disk.GetByte();
                        }
                    }

                    // ADPCM decoder
                    for(int su = 0; su < 4; su++)
                    {
                        uint16_t gain = pow(2, 8 - range[su]);
                        float k = k0[filter[su]] + k1[filter[su]];
                        if(ms) // stereo
                        {
                            for(uint8_t ss = 0; ss < 28; ss++)
                                if(su & 1)
                                    right.push_back(SD[su][ss] * gain + k);
                                else
                                    left.push_back(SD[su][ss] * gain + k);
                        }
                        else // mono
                        {
                            for(uint8_t ss = 0; ss < 28; ss++)
                                left.push_back(SD[su][ss] * gain + k);
                        }
                    }
                }
                wavHeader.frequency = 37800;
            }
            else // Level B and C (4 bits per sample)
            {
                for(uint8_t sg = 0; sg < 18; sg++)
                {
                    uint8_t s[8];
                    uint8_t range[8];
                    uint8_t filter[8];
                    uint8_t SD[8][28]; // sound data

                    disk.Seek(4, std::ios::cur);
                    disk.Read((char*)s, 8);
                    disk.Seek(4, std::ios::cur);
                    for(uint8_t i = 0; i < 8; i++)
                    {
                        range[i] = s[i] & 0x0F;
                        filter[i] = s[i] >> 4;
                    }

                    for(uint8_t ss = 0; ss < 28; ss++) // sound sample
                    {
                        for(uint8_t su = 0; su < 8; su += 2)
                        {
                            const uint8_t SB = disk.GetByte();
                            SD[su][ss] = SB & 0x0F;
                            SD[su+1][ss] = SB >> 4;
                        }
                    }

                    // ADPCM decoder
                    for(int su = 0; su < 8; su++)
                    {
                        uint16_t gain = pow(2, 12 - range[su]);
                        float k = k0[filter[su]] + k1[filter[su]];
                        if(ms) // stereo
                        {
                            for(uint8_t ss = 0; ss < 28; ss++)
                                if(su & 1)
                                    right.push_back(SD[su][ss] * gain + k);
                                else
                                    left.push_back(SD[su][ss] * gain + k);
                        }
                        else // mono
                        {
                            for(uint8_t ss = 0; ss < 28; ss++)
                                left.push_back(SD[su][ss] * gain + k);
                        }
                    }
                }
                wavHeader.frequency = fs ? 18900 : 37800;
            }
            wavHeader.bitsPerSample = 16;
            wavHeader.channelNumber = ms + 1;

            disk.GotoNextSector();
        }

        if(left.size())
        {
            std::ofstream out(directoryPath + filename + '_' + std::to_string(channel) + ".wav", std::ios::binary | std::ios::out);

            uint16_t bytePerBloc = wavHeader.channelNumber * wavHeader.bitsPerSample / 8;
            uint32_t bytePerSec = wavHeader.frequency * bytePerBloc;
            uint32_t dataSize = left.size()*2 + right.size()*2;
            uint32_t wavSize = 36 + dataSize;

            out.write("RIFF", 4);
            out.write((char*)&wavSize, 4);
            out.write("WAVE", 4);
            out.write("fmt ", 4);
            out.write("\x10\0\0\0", 4);
            out.write("\1\0", 2); // audio format

            out.write((char*)&wavHeader.channelNumber, 2);
            out.write((char*)&wavHeader.frequency, 4);
            out.write((char*)&bytePerSec, 4);
            out.write((char*)&bytePerBloc, 2);
            out.write((char*)&wavHeader.bitsPerSample, 2);
            out.write("data", 4);
            out.write((char*)&dataSize, 4);

            if(right.size()) // stereo
            {
                for(uint32_t i = 0; i < left.size() && i < right.size(); i++)
                {
                    out.write((char*)&left[i], 2);
                    out.write((char*)&right[i], 2);
                }
            }
            else // mono
            {
                std::ostream_iterator<int16_t> outit(out);
                std::copy(left.begin(), left.end(), outit);
            }

            out.close();
        }
    }
    disk.Seek(pos);
}

/**
* Writes the content of the file (including empty sectors) in {directoryPath} + this->filename.
**/
void CDIFile::ExportFile(std::string directoryPath)
{
    const uint32_t pos = disk.Tell();

    std::ofstream out(directoryPath + filename, std::ios::out | std::ios::binary);

    uint32_t size = 0;
    char const * const data = GetFileContent(size);
    if(data && size)
        out.write(data, size);
    delete[] data;

    out.close();
    disk.Seek(pos);
}

/**
* The parameter is a pointer that, if not null, will contain
* the size of the file content.
* Remember to delete the returned pointer (allocated with new[])
**/
char* CDIFile::GetFileContent(uint32_t& size)
{
    const uint32_t pos = disk.Tell();

    uint32_t readSize = (double)filesize / 2048.0 * 2324.0;
    char* data = new (std::nothrow) char[readSize];
    if(data == nullptr)
    {
        wxMessageBox("Could not allocate memory to export file " + filename);
        return nullptr;
    }

    readSize = filesize;
    disk.GotoLBN(fileLBN);
    disk.GetData(data, readSize, true);
    size = readSize;

    disk.Seek(pos);
    return data;
}
