#include "CDIFile.hpp"
#include "CDIDisc.hpp"
#include "common/Audio.hpp"
#include "common/utils.hpp"
#include "common/Video.hpp"
#include "cores/MCD212/MCD212.hpp"

#include <wx/bitmap.h>

#include <array>
#include <fstream>
#include <vector>

static constexpr uint8_t codingLookUp[16] = {
    CLUT4, CLUT7, CLUT8, OFF,
    OFF, DYUV, RGB555, RGB555,
    OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF,
};

CDIFile::CDIFile(CDIDisc& cdidisc, uint32_t lbn, uint32_t filesize, uint8_t namesize, std::string filename, uint16_t attr, uint8_t filenumber, uint16_t parentRelpos) :
    disc(cdidisc),
    LBN(lbn),
    size(filesize),
    nameSize(namesize),
    name(filename),
    attributes(attr),
    number(filenumber),
    parent(parentRelpos)
{}

static std::string getAudioLevel(const bool bps, const uint32_t fs)
{
    if(fs == 18900)
        return "C";
    if(bps)
        return "A";
    return "B";
}

/** \brief Export the audio data of the file.
 *
 * \param  directoryPath Path to the directory where the files will be written.
 *
 * Converts and writes the audio data from the ROM to 16-bit PCM.
 * Each channel and logical records are exported individualy.
 */
void CDIFile::ExportAudio(const std::string& directoryPath)
{
    uint32_t pos = disc.Tell();
    int maxChannel = 0;

    for(int channel = 0; channel <= maxChannel; channel++)
    {
        Audio::resetAudioFiltersDelay();
        Audio::WAVHeader wavHeader;
        std::vector<int16_t> left;
        std::vector<int16_t> right;
        uint8_t bps = 3;

        disc.GotoLBN(LBN);

        uint8_t record = 0;
        int32_t sizeLeft = size;
        while(sizeLeft > 0)
        {
            sizeLeft -= (sizeLeft < 2048) ? sizeLeft : 2048;
            if(disc.subheader.channelNumber > maxChannel)
                maxChannel = disc.subheader.channelNumber;

            if(!(disc.subheader.submode & cdia) || disc.subheader.channelNumber != channel)
            {
                disc.GotoNextSector();
                continue;
            }

            // bool  emph = disc.subheader.codingInformation & Audio::CodingInformation::emphasis;
                         bps = (disc.subheader.codingInformation & Audio::CodingInformation::bps) >> 4;
            const uint8_t sf = (disc.subheader.codingInformation & Audio::CodingInformation::sf) >> 2;
            const uint8_t ms =  disc.subheader.codingInformation & Audio::CodingInformation::ms;

            if(bps > 1 || sf > 1 || ms > 1) // ignore reserved values
            {
                disc.GotoNextSector();
                continue;
            }

            uint8_t data[2304];
            disc.GetRaw(data, 2304);
            Audio::decodeAudioSector(bps, ms, data, left, right);

            wavHeader.channelNumber = ms + 1;
            wavHeader.frequency = sf ? 18900 : 37800;

            if(disc.subheader.submode & cdieor)
            {
                std::ofstream out(directoryPath + name + '_' + std::to_string(channel) + "_" + std::to_string(record++) + "_" + getAudioLevel(bps, wavHeader.frequency) + ".wav", std::ios::binary | std::ios::out);
                Audio::writeWAV(out, wavHeader, left, right);
                out.close();
                left.clear();
                right.clear();
            }

            disc.GotoNextSector();
        }

        if(left.size())
        {
            std::ofstream out(directoryPath + name + '_' + std::to_string(channel) + "_" + std::to_string(record) + "_" + getAudioLevel(bps, wavHeader.frequency) + ".wav", std::ios::binary | std::ios::out);
            Audio::writeWAV(out, wavHeader, left, right);
            out.close();
        }
    }

    disc.Seek(pos);
}

/** \brief Write the content of the file (as stored in the ROM) on disc.
 *
 * \param  directoryPath Path to the directory where the file will be written (must end with a '/').
 */
void CDIFile::ExportFile(const std::string& directoryPath)
{
    const uint32_t pos = disc.Tell();

    std::ofstream out(directoryPath + name, std::ios::out | std::ios::binary);

    uint32_t size = 0;
    uint8_t const * const data = GetContent(size);
    if(data && size)
        out.write((char*)data, size);
    delete[] data;

    out.close();
    disc.Seek(pos);
}

/** \brief Export the video data of the file.
 *
 * \param  directoryPath Path to the directory where the files will be written.
 *
 * Converts and writes the video data from the ROM.
 * Each channel and logical records are exported individualy.
 */
void CDIFile::ExportVideo(const std::string& directoryPath)
{
    uint32_t pos = disc.Tell();
    int maxChannel = 0;

    for(int channel = 0; channel <= maxChannel; channel++)
    {
        uint16_t width = 0, height = 0, y = 0;
        uint8_t coding = 0;
        uint8_t pixels[768 * 560 * 4] = {0};
        std::vector<uint8_t> data;

        disc.GotoLBN(LBN);

        uint8_t record = 0;
        int32_t sizeLeft = size;
        while(sizeLeft > 0)
        {
            sizeLeft -= (sizeLeft < 2048) ? sizeLeft : 2048;
            if(disc.subheader.channelNumber > maxChannel)
                maxChannel = disc.subheader.channelNumber;

            if(disc.subheader.submode & cdid) // Get CLUT table from a sector before the video data
            {
                uint8_t data[2048];
                disc.GetRaw(data, 2048);

                char* clut = (char*)subarrayOfArray(data, 2048, "cluts", 5);
                if(clut != nullptr)
                {
                    disc.Seek(-2026, std::ios::cur);
                    const uint16_t offset = disc.GetWord();

                    disc.Seek(0x2A, std::ios::cur);
                    std::string cluts = disc.GetString(5);

                    if(cluts.compare("cluts") != 0)
                        continue;

                    disc.Seek(offset - 0x2A - 0x16 - 5 - 2, std::ios::cur);
                    for(int bank = 0; bank < 256; bank += 64)
                        for(int i = 0; i < 64; i++)
                        {
                            uint8_t addr = disc.GetByte();
                            if(addr == 0)
                            {
                                bank = 256 * 3;
                                break;
                            }
                            addr -= 0x80;
                            Video::CLUT[bank + addr]  = disc.GetByte() << 16;
                            Video::CLUT[bank + addr] |= disc.GetByte() << 8;
                            Video::CLUT[bank + addr] |= disc.GetByte();
                        }
                }
                else // simply copy the first 128 colors
                {
                    for(int i = 0, j = 0; i < 128*3; j++)
                    {
                        Video::CLUT[j]  = data[i++] << 16;
                        Video::CLUT[j] |= data[i++] << 8;
                        Video::CLUT[j] |= data[i++];
                    }
                }

                disc.GotoNextSector();
                continue;
            }

            if(!(disc.subheader.submode & cdiv) || disc.subheader.channelNumber != channel)
            {
                disc.GotoNextSector();
                continue;
            }

            bool ascf = disc.subheader.codingInformation & Video::CodingInformation::ascf;
            // bool eolf = disc.subheader.codingInformation & Video::CodingInformation::eolf;
            uint8_t resolution  = (disc.subheader.codingInformation & Video::CodingInformation::resolution) >> 2;
            coding = disc.subheader.codingInformation & Video::CodingInformation::coding;

            if(ascf || coding > 7 || resolution == 2)
            {
                disc.GotoNextSector();
                continue;
            }

            std::array<uint8_t, 2324> d;

            width = resolution == 0 ? 384 : 768;
            height = resolution == 3 ? 480 : 242;

            disc.GetRaw(d.data(), d.size());
            data.insert(data.end(), d.begin(), d.end());

            disc.GotoNextSector();
        }

        size_t index = 0;
        if(coding == 3 || coding == 4)
        {
            while(index < data.size())
            {
                index += Video::decodeRunLengthLine(&pixels[width * 4 * y], width, &data[index], Video::CLUT, coding & 0x3);
                y++;
                if(y >= height)
                {
                    uint8_t* pix = new uint8_t[width * height * 3];
                    Video::splitARGB(pixels, width * height * 4, nullptr, pix);
                    wxImage(width, height, pix, true).SaveFile(directoryPath + name + "_" + std::to_string(channel) + "_" + std::to_string(record++) + ".bmp", wxBITMAP_TYPE_BMP);
                    delete[] pix;
                    y = 0;
                }
            }
        }
        else
        {
            while(index < data.size())
            {
                index += Video::decodeBitmapLine(&pixels[width * 4 * y], width, nullptr, &data[index], Video::CLUT, 0, codingLookUp[coding]);
                y++;
                if(y >= height)
                {
                    uint8_t* pix = new uint8_t[width * height * 3];
                    Video::splitARGB(pixels, width * height * 4, nullptr, pix);
                    wxImage(width, height, pix, true).SaveFile(directoryPath + name + "_" + std::to_string(channel) + "_" + std::to_string(record++) + ".bmp", wxBITMAP_TYPE_BMP);
                    delete[] pix;
                    y = 0;
                }
            }
        }

        if(y > 0)
        {
            uint8_t* pix = new uint8_t[width * height * 4];
            Video::splitARGB(pixels, width * height * 4, nullptr, pix);
            wxImage(width, height, pix, true).SaveFile(directoryPath + name + "_" + std::to_string(channel) + "_" + std::to_string(record++) + ".bmp", wxBITMAP_TYPE_BMP);
            delete[] pix;
            y = 0;
        }
    }

    disc.Seek(pos);
}

/** \brief Get the file content.
 *
 * \param  size A reference to a uint32_t that will contain the size of the returned array.
 * \return An array containing the file content, or nullptr if memory allocation failed. It is the caller's responsability to delete the returned array (allocated with new[]).
 */
uint8_t* CDIFile::GetContent(uint32_t& size)
{
    const uint32_t pos = disc.Tell();

    uint32_t readSize = (double)this->size / 2048.0 * 2324.0;
    uint8_t* data = new (std::nothrow) uint8_t[readSize];
    if(data == nullptr)
        return nullptr;

    readSize = this->size;
    disc.GotoLBN(LBN);
    disc.GetData(data, readSize, true);
    size = readSize;

    disc.Seek(pos);
    return data;
}
