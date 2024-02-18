#include "CDIFile.hpp"
#include "CDIDisc.hpp"
#include "common/Audio.hpp"
#include "common/utils.hpp"
#include "common/Video.hpp"

#include <wx/bitmap.h>

#include <array>
#include <fstream>
#include <vector>

/** \brief The max number of channels (Green book Appendix II.1.2). */
static constexpr size_t MAX_CHANNEL_NUMBER = 32;
/** \brief The max number of channels on audio sectors (Green book Appendix II.1.2). */
static constexpr size_t MAX_AUDIO_CHANNEL_NUMBER = 16;

static constexpr Video::ImageCodingMethod codingLookUp[16] = {
    ICM(CLUT4), ICM(CLUT7), ICM(CLUT8), ICM(OFF),
    ICM(OFF), ICM(DYUV), ICM(RGB555), ICM(RGB555),
    ICM(OFF), ICM(OFF), ICM(OFF), ICM(OFF),
    ICM(OFF), ICM(OFF), ICM(OFF), ICM(OFF),
};

CDIFile::CDIFile(CDIDisc& cdidisc, uint32_t lbn, uint32_t filesize, uint8_t namesize, std::string filename, uint16_t attr, uint8_t filenumber, uint16_t parentRelpos)
    : LBN(lbn)
    , size(filesize)
    , nameSize(namesize)
    , name(filename)
    , attributes(attr)
    , number(filenumber)
    , parent(parentRelpos)
    , disc(cdidisc)
{}

static std::string codingMethodToString(const uint8_t method)
{
    switch(method)
    {
    case 0: return "CLUT4";
    case 1: return "CLUT7";
    case 2: return "CLUT8";
    case 3: return "RL3";
    case 4: return "RL7";
    case 5: return "DYUV";
    case 6: return "RGB555lower";
    case 7: return "RGB555upper";
    case 8: return "QHY";
    default: return "Reserved";
    }
}

static std::string eolfToString(const bool eolf)
{
    if(eolf)
        return "Odd";
    else
        return "Even";
}

static std::string resolutionToString(const uint8_t resolution)
{
    switch(resolution)
    {
    case 0: return "Normal";
    case 1: return "Double";
    case 3: return "High";
    default: return "Reserved";
    }
}

/** \brief Clears \p data after being written. */
static void writeRawVideo(std::vector<uint8_t>& data, const std::string& basename, int channel, int record, bool ascf, bool eolf, uint8_t resolution, uint8_t coding)
{
    std::ofstream out(basename + "_" + std::to_string(channel) + "_" + std::to_string(record) + \
                      (ascf ? "_ASCF_" : "_") + \
                      eolfToString(eolf) + "_" + \
                      resolutionToString(resolution) + "_" + \
                      codingMethodToString(coding), std::ios::out | std::ios::binary);
    out.write((char*)data.data(), data.size());
    out.close();
    data.clear();
}

/** \brief Exports the audio data of the file.
 *
 * \param  directoryPath Path to the directory where the files will be written. Must end with a '/' (or '\' on Windows).
 *
 * Converts and writes the audio data from the disc to 16-bit PCM.
 * Each channel and logical records are exported individualy.
 */
void CDIFile::ExportAudio(const std::string& directoryPath) const
{
    struct AudioInfo
    {
        uint8_t bps;
        uint8_t sf;
        uint8_t ms;
        uint8_t record;
        Audio::SamplesDelay delay;
        std::vector<int16_t> left; // If left if empty, right must be empty.
        std::vector<int16_t> right;
    };
    std::array<AudioInfo, MAX_AUDIO_CHANNEL_NUMBER> audio{};

    ForEachSector([&] (const CDISector& sector) {
        if((sector.subheader.submode & cdia) == 0)
            return;

        // Green Book IV.3.2.4
//         const bool emph  = bit<6>(disc.m_subheader.codingInformation);
        const uint8_t bps = bits<4, 5>(disc.m_subheader.codingInformation);
        const uint8_t sf = bits<2, 3>(disc.m_subheader.codingInformation);
        const uint8_t ms = bits<0, 1>(disc.m_subheader.codingInformation);

        if(bps > 1 || sf > 1 || ms > 1) // ignore reserved values.
            return;

        AudioInfo& a = audio[sector.subheader.channelNumber];
        if(!a.left.empty() &&
           (a.bps != bps ||
            a.sf != sf ||
            a.ms != ms))
            Audio::writeWAV(directoryPath + name, a.left, a.right, sector.subheader.channelNumber, a.record++, a.bps, a.sf, a.ms);

        a.bps = bps;
        a.sf = sf;
        a.ms = ms;

        Audio::decodeAudioSector(a.delay, bps, ms, sector.data.data(), a.left, a.right);

        if(sector.subheader.submode & cdieor)
            Audio::writeWAV(directoryPath + name, a.left, a.right, sector.subheader.channelNumber, a.record++, a.bps, a.sf, a.ms);
    });

    int channel = 0;
    for(AudioInfo& a : audio)
    {
        if(!a.left.empty())
            Audio::writeWAV(directoryPath + name, a.left, a.right, channel, a.record++, a.bps, a.sf, a.ms);
        channel++;
    }
}

/** \brief Exports the content of the file (as stored in the disc).
 *
 * \param  directoryPath Path to the directory where the file will be written. Must end with a '/' (or '\' on Windows).
 */
void CDIFile::ExportFile(const std::string& directoryPath) const
{
    const uint32_t pos = disc.Tell();

    std::ofstream out(directoryPath + name, std::ios::out | std::ios::binary);

    std::vector<uint8_t> data = GetContent();
    if(!data.empty())
        out.write(reinterpret_cast<char*>(data.data()), data.size());

    out.close();
    disc.Seek(pos);
}

/** \brief Exports the video data of the file.
 *
 * \param  directoryPath Path to the directory where the files will be written. Must end with a '/' (or '\' on Windows).
 *
 * Converts and writes the video data from the disc.
 * Each channel are exported individualy.
 */
void CDIFile::ExportVideo(const std::string& directoryPath) const
{
    uint32_t pos = disc.Tell();
    int maxChannel = 0;
    std::array<uint32_t, 256> CLUT{};

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
            if(disc.m_subheader.channelNumber > maxChannel)
                maxChannel = disc.m_subheader.channelNumber;

            if(disc.m_subheader.submode & cdid) // Get CLUT table from a sector before the video data
            {
                std::array<uint8_t, 2048> d;
                disc.GetRaw(d);

                // const_cast is safe because `d` is not const.
                char* clut = const_cast<char*>(as<const char*>(subarrayOfArray(d.data(), d.size(), "cluts", 5)));
                if(clut != nullptr)
                {
                    // Retro engineering from Link: The Faces of Evil.
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
                            CLUT[bank + addr]  = as<uint32_t>(disc.GetByte()) << 16;
                            CLUT[bank + addr] |= as<uint32_t>(disc.GetByte()) << 8;
                            CLUT[bank + addr] |= disc.GetByte();
                        }
                }
                else // simply copy the first 128 colors
                {
                    for(int i = 0, j = 0; j < 128; j++)
                    {
                        CLUT[j]  = as<uint32_t>(d[i++]) << 16;
                        CLUT[j] |= as<uint32_t>(d[i++]) << 8;
                        CLUT[j] |= d[i++];
                    }
                }

                disc.GotoNextSector();
                continue;
            }

            if(!(disc.m_subheader.submode & cdiv) || disc.m_subheader.channelNumber != channel)
            {
                disc.GotoNextSector();
                continue;
            }

            // Green Book V.6.3.1
            bool ascf = bit<7>(disc.m_subheader.codingInformation);
//            bool eolf = bit<6>(disc.m_subheader.codingInformation);
            uint8_t resolution = bits<4, 5>(disc.m_subheader.codingInformation);
            coding = bits<0, 3>(disc.m_subheader.codingInformation);

            if(ascf || coding > 7 || resolution == 2)
            {
                disc.GotoNextSector();
                continue;
            }

            std::array<uint8_t, 2324> d;

            width = resolution == 0 ? 384 : 768;
            height = resolution == 3 ? 480 : 242;

            disc.GetRaw(d);
            data.insert(data.end(), d.begin(), d.end());

            disc.GotoNextSector();
        }

        size_t index = 0;
        if(coding == 3 || coding == 4)
        {
            while(index < data.size())
            {
                index += Video::decodeRunLengthLine(&pixels[width * 4 * y], &data[index], width, CLUT.data(), coding & 0x3);
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
                index += Video::decodeBitmapLine(&pixels[width * 4 * y], nullptr, &data[index], width, CLUT.data(), 0x00108080, codingLookUp[coding]);
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

/** \brief Exports the raw video data of the file (unconverted).
 *
 * \param  directoryPath Path to the directory where the files will be written. Must end with a '/' (or '\' on Windows).
 *
 * Only writes the raw video data from the file.
 * Each channel and logical records are exported individualy.
 */
void CDIFile::ExportRawVideo(const std::string& directoryPath) const
{
    struct VideoInfo
    {
        bool ascf;
        bool eolf;
        uint8_t resolution;
        uint8_t coding;
        uint8_t record;
        std::vector<uint8_t> data;
    };
    std::array<VideoInfo, MAX_CHANNEL_NUMBER> video{};

    ForEachSector([&] (const CDISector& sector) {
        if((sector.subheader.submode & cdiv) == 0)
            return;

        const bool ascf = bit<7>(sector.subheader.codingInformation);
        const bool eolf = bit<6>(sector.subheader.codingInformation);
        const uint8_t resolution = bits<4, 5>(sector.subheader.codingInformation);
        const uint8_t coding = bits<0, 3>(sector.subheader.codingInformation);

        VideoInfo& v = video[sector.subheader.channelNumber];
        if(!v.data.empty() &&
           (v.ascf != ascf ||
            v.eolf != eolf ||
            v.resolution != resolution ||
            v.coding != coding))
            writeRawVideo(v.data, directoryPath + name, sector.subheader.channelNumber, v.record++, v.ascf, v.eolf, v.resolution, v.coding);

        v.ascf = ascf;
        v.eolf = eolf;
        v.resolution = resolution;
        v.coding = coding;

        v.data.insert(v.data.end(), sector.data.begin(), sector.data.end());

        if(sector.subheader.submode & cdieor)
            writeRawVideo(v.data, directoryPath + name, sector.subheader.channelNumber, v.record++, v.ascf, v.eolf, v.resolution, v.coding);
    });

    int channel = 0;
    for(VideoInfo& v : video)
    {
        if(!v.data.empty())
            writeRawVideo(v.data, directoryPath + name, channel, v.record++, v.ascf, v.eolf, v.resolution, v.coding);
        channel++;
    }
}

/** \brief Get the file content.
 * \return A vector containing the file content.
 */
std::vector<uint8_t> CDIFile::GetContent() const
{
    uint32_t readSize = as<uint64_t>(size) * 2324 / 2048;
    std::vector<uint8_t> data;
    data.reserve(readSize);

    readSize = size;
    ForEachSector([&] (const CDISector& sector) {
        const uint16_t sectorSize = sector.GetSectorDataSize();
        const uint32_t sz = readSize < sectorSize ? readSize : sectorSize;
        // std::vector<uint8_t>::const_iterator begin = sector.data.begin();
        decltype(sector.data)::const_iterator begin = sector.data.begin();

        data.insert(data.end(), begin, begin + sz);
    });

    return data;
}

/** \brief Calls the given function on each sector or the file.
 * \param f The function to call.
 */
void CDIFile::ForEachSector(std::function<void(const CDISector&)> f) const
{
    disc.ForEachFileSector(LBN, std::move(f));
}
