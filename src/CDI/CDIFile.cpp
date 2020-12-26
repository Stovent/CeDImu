#include "CDIFile.hpp"
#include "CDIDisc.hpp"
#include "../utils.hpp"
#include "../common/Audio.hpp"
#include "../common/Video.hpp"

#include <wx/msgdlg.h>
#include <wx/bitmap.h>

#include <fstream>
#include <vector>

CDIFile::CDIFile(CDIDisc& cdidisc, uint32_t lbn, uint32_t size, uint8_t namesize, std::string name, uint16_t attr, uint8_t filenumber, uint16_t parentRelpos) :
    disc(cdidisc),
    fileLBN(lbn),
    filesize(size),
    nameSize(namesize),
    filename(name),
    attributes(attr),
    fileNumber(filenumber),
    parent(parentRelpos)
{}

/** \brief Export the audio data of the file.
 *
 * \param  directoryPath Path to the directory where the files will be written.
 *
 * Converts and writes the audio data from the ROM to 16-bit PCM.
 * Each channel and logical records are exported individualy.
 * Special thanks to this thread (http://www.cdinteractive.co.uk/forums/cdinteractive/viewtopic.php?t=3191)
 * for making me understand how the k0 and k1 filters worked in ADCPM decoder
 */
void CDIFile::ExportAudio(std::string directoryPath)
{
    uint32_t pos = disc.Tell();
    int maxChannel = 0;

    for(int channel = 0; channel <= maxChannel; channel++)
    {
        Audio::resetAudioFiltersDelay();
        Audio::WAVHeader wavHeader;
        std::vector<int16_t> left;
        std::vector<int16_t> right;

        disc.GotoLBN(fileLBN);

        uint8_t record = 0;
        int32_t sizeLeft = filesize;
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

            // bool emph = disc.subheader.codingInformation & Audio::CodingInformation::emphasis;
            bool bps = disc.subheader.codingInformation & Audio::CodingInformation::bps;
            bool sf = disc.subheader.codingInformation & Audio::CodingInformation::sf;
            bool ms = disc.subheader.codingInformation & Audio::CodingInformation::ms;

            uint8_t data[2304];
            disc.GetRaw((char*)data, 2304);
            Audio::decodeAudioSector(bps, ms, data, left, right);

            wavHeader.channelNumber = ms + 1;
            wavHeader.frequency = bps ? 37800 : (sf ? 18900 : 37800);

            if(disc.subheader.submode & cdieor)
            {
                std::ofstream out(directoryPath + filename + '_' + std::to_string(channel) + "_" + std::to_string(record++) + ".wav", std::ios::binary | std::ios::out);
                Audio::writeWAV(out, wavHeader, left, right);
                out.close();
                left.clear();
                right.clear();
            }

            disc.GotoNextSector();
        }

        if(left.size())
        {
            std::ofstream out(directoryPath + filename + '_' + std::to_string(channel) + "_" + std::to_string(record) + ".wav", std::ios::binary | std::ios::out);
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
void CDIFile::ExportFile(std::string directoryPath)
{
    const uint32_t pos = disc.Tell();

    std::ofstream out(directoryPath + filename, std::ios::out | std::ios::binary);

    uint32_t size = 0;
    char const * const data = GetFileContent(size);
    if(data && size)
        out.write(data, size);
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
void CDIFile::ExportVideo(std::string directoryPath)
{
    uint32_t pos = disc.Tell();
    int maxChannel = 0;

    for(int channel = 0; channel <= maxChannel; channel++)
    {
        uint16_t width, height, y = 0;
        uint8_t pixels[768 * 560 * 3] = {0};

        disc.GotoLBN(fileLBN);

        uint8_t record = 0;
        int32_t sizeLeft = filesize;
        while(sizeLeft > 0)
        {
            sizeLeft -= (sizeLeft < 2048) ? sizeLeft : 2048;
            if(disc.subheader.channelNumber > maxChannel)
                maxChannel = disc.subheader.channelNumber;

            if(disc.subheader.submode & cdid) // Get CLUT table from a sector before the video data
            {
                uint8_t data[2048];
                disc.GetRaw((char*)data, 2048);

                char* clut = (char*)subarrayOfArray(data, 2048, "cluts", 5);
                if(clut != NULL)
                {
                    disc.Seek(-2026, std::ios::cur);
                    const uint16_t offset = disc.GetWord();

                    disc.Seek(0x2A, std::ios::cur);
                    std::string cluts = disc.GetString(5);

                    if(cluts.compare("cluts") != 0)
                        continue;

                    disc.Seek(offset - 0x2A - 0x16 - 5 - 2, std::ios::cur);
                    for(int bank = 0; bank < 256 * 3; bank += 64 * 3)
                        for(int i = 0; i < 64; i++)
                        {
                            uint8_t addr = disc.GetByte();
                            if(addr == 0)
                            {
                                bank = 256 * 3;
                                break;
                            }
                            addr -= 0x80;
                            Video::CLUT[bank + 3 * addr] = disc.GetByte();
                            Video::CLUT[bank + 3 * addr + 1] = disc.GetByte();
                            Video::CLUT[bank + 3 * addr + 2] = disc.GetByte();
                        }
                }
                else // simply copy the first 128 colors
                {
                    for(int i = 0, j = 0; i < 128*3; j++)
                    {
                        Video::CLUT[j] = data[i] << 16; i++;
                        Video::CLUT[j] |= data[i] << 8; i++;
                        Video::CLUT[j] |= data[i]; i++;
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
            uint8_t coding = disc.subheader.codingInformation & Video::CodingInformation::coding;

            if(ascf)
                continue;

            uint8_t data[2324];
            uint16_t index = 0;

            width = resolution == 0 ? 360 : 720;
            height = resolution == 3 ? 490 : 245;

            disc.GetRaw((char*)data, 2324);

            while(index < 2324)
            {
                if(coding == 4)
                {
                    index += Video::DecodeRunLengthLine(&pixels[y * 3 * width], width, Video::CLUT, &data[index], 0); // TODO: this should be ARGB.
                    y++;
                    if(y >= height)
                    {
                        uint8_t* pix = new uint8_t[width * height * 3];
                        memcpy(pix, pixels, width * height * 3);
                        wxBitmap(wxImage(width, height, pix, true), 24).SaveFile(directoryPath + filename + "_" + std::to_string(channel) + "_" + std::to_string(record++) + ".bmp", wxBITMAP_TYPE_BMP);
                        delete[] pix;
                        y = 0;
                    }
                }
                else
                {
//                    index += Video::DecodeBitmapLine(&data[index], &pixels[y * 3 * width], width, coding);
                    y++;
                    if(y >= height)
                    {
                        uint8_t* pix = new uint8_t[width * height * 3];
                        memcpy(pix, pixels, width * height * 3);
                        wxBitmap(wxImage(width, height, pix, true), 24).SaveFile(directoryPath + filename + "_" + std::to_string(channel) + "_" + std::to_string(record++) + ".bmp", wxBITMAP_TYPE_BMP);
                        delete[] pix;
                        y = 0;
                    }
                }
            }

            disc.GotoNextSector();
        }
    }

    disc.Seek(pos);
}

/** \brief Get the file content.
 *
 * \param  size A reference to a uint32_t that will contain the size of the returned array.
 * \return An array containing the file content, or nullptr if memory allocation failed. It is the caller's responsability to delete the returned array (allocated with new[]).
 */
char* CDIFile::GetFileContent(uint32_t& size)
{
    const uint32_t pos = disc.Tell();

    uint32_t readSize = (double)filesize / 2048.0 * 2324.0;
    char* data = new (std::nothrow) char[readSize];
    if(data == nullptr)
    {
        wxMessageBox("Could not allocate memory to export file " + filename);
        return nullptr;
    }

    readSize = filesize;
    disc.GotoLBN(fileLBN);
    disc.GetData(data, readSize, true);
    size = readSize;

    disc.Seek(pos);
    return data;
}
