#include "CDIFile.hpp"

#include <fstream>
#include <vector>

#include "../common/audio.hpp"

#include <wx/msgdlg.h>

CDIFile::CDIFile(CDIDisk& cdidisk, uint32_t lbn, uint32_t size, uint8_t namesize, std::string name, uint16_t attr, uint8_t filenumber, uint16_t parentRelpos) :
    disk(cdidisk),
    fileLBN(lbn),
    filesize(size),
    nameSize(namesize),
    filename(name),
    attributes(attr),
    fileNumber(filenumber),
    parent(parentRelpos)
{}

static void writeWAV(std::ofstream& out, const WAVHeader& wavHeader, const std::vector<int16_t>& left, const std::vector<int16_t>& right)
{
    uint16_t bytePerBloc = wavHeader.channelNumber * 2;
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
    out.write("\x10\0", 2);
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
        out.write((char*)&left[0], left.size() * 2);
    }
}

/**
* Converts and writes the audio data from the disk to 16-bit PCM on a
* file in the directory {directoryPath}, each channel is exported individualy.
* Special thanks to this thread (http://www.cdinteractive.co.uk/forums/cdinteractive/viewtopic.php?t=3191)
* for making me understand how the k0 and k1 filters worked in ADCPM decoder
**/
void CDIFile::ExportAudio(std::string directoryPath)
{
    uint32_t pos = disk.Tell();

    for(int channel = 0; channel < 16; channel++)
    {
        WAVHeader wavHeader;
        std::vector<int16_t> left;
        std::vector<int16_t> right;

        disk.GotoLBN(fileLBN);

        uint8_t record = 0;
        int32_t sizeLeft = filesize;
        while(sizeLeft > 0)
        {
            sizeLeft -= (sizeLeft < 2048) ? sizeLeft : 2048;

            if(!(disk.subheader.Submode & cdia) || disk.subheader.ChannelNumber != channel)
            {
                disk.GotoNextSector();
                continue;
            }

            // bool emph = disk.subheader.CodingInformation & AudioCodingInformation::emphasis;
            bool bps = disk.subheader.CodingInformation & AudioCodingInformation::bps;
            bool sf = disk.subheader.CodingInformation & AudioCodingInformation::sf;
            bool ms = disk.subheader.CodingInformation & AudioCodingInformation::ms;

            uint8_t data[2304];
            disk.Read((char*)data, 2304);
            decodeAudioSector(bps, ms, data, left, right);

            wavHeader.channelNumber = ms + 1;
            wavHeader.frequency = bps ? 37800 : (sf ? 18900 : 37800);

            if(disk.subheader.Submode & cdieor)
            {
                std::ofstream out(directoryPath + filename + '_' + std::to_string(channel) + "_" + std::to_string(record++) + ".wav", std::ios::binary | std::ios::out);
                writeWAV(out, wavHeader, left, right);
                out.close();
                left.clear();
                right.clear();
            }

            disk.GotoNextSector();
        }

        if(left.size())
        {
            std::ofstream out(directoryPath + filename + '_' + std::to_string(channel) + "_" + std::to_string(record) + ".wav", std::ios::binary | std::ios::out);
            writeWAV(out, wavHeader, left, right);
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
