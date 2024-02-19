#include "export.hpp"
#include "CDI/common/utils.hpp"
#include "CDI/common/Video.hpp"

#include <wx/bitmap.h>

#include <array>
#include <filesystem>

static constexpr Video::ImageCodingMethod codingLookUp[16] = {
    ICM(CLUT4), ICM(CLUT7), ICM(CLUT8), ICM(OFF),
    ICM(OFF), ICM(DYUV), ICM(RGB555), ICM(RGB555),
    ICM(OFF), ICM(OFF), ICM(OFF), ICM(OFF),
    ICM(OFF), ICM(OFF), ICM(OFF), ICM(OFF),
};

/** \brief Tries to decode the video data of the sectors to a file.
 * \throw std::filesystem::filesystem_error if it cannot create directories.
 *
 * Note: this function actually doesn't really work. At least on LINK - The Faces of Evil the exported images are
 * often of the wrong color and displaced horizontally and vertically, but the shapes are visible.
 *
 * It still exists to show how to use these functions.
 */
void exportVideo(CDIDisc& disc, std::string exportDir)
{
    const std::string path = exportDir + "/" + disc.m_gameName + "/video/";

    disc.ForEachFile([&] (std::string_view fileDir, const CDIFile& file) {
        struct VideoInfo
        {
            uint8_t resolution;
            uint8_t coding;
            size_t record;
            std::vector<uint8_t> data;
        };
        std::array<VideoInfo, MAX_CHANNEL_NUMBER> video{};
        std::array<uint32_t, 256> CLUT{};

        file.ForEachSector([&] (const CDISector& sector) {
            if(sector.subheader.submode & cdid) // Get CLUT table from a sector before the video data.
            {
                const char* clut = as<const char*>(subarrayOfArray(sector.data.data(), sector.data.size(), "cluts", 5));
                if(clut != nullptr)
                {
                    // Retro engineered from LINK - The Faces of Evil.
                    std::string cluts = std::string(reinterpret_cast<const char*>(&sector.data[66]), 5);
                    if(cluts.compare("cluts") != 0)
                        return;

                    uint16_t offset = GET_ARRAY16(sector.data, 22);
                    for(int bank = 0; bank < 256; bank += 64)
                        for(int i = 0; i < 64; i++)
                        {
                            uint8_t addr = sector.data[offset];
                            if(addr == 0)
                            {
                                bank = 256 * 3;
                                break;
                            }
                            addr -= 0x80;
                            CLUT[bank + addr] = GET_ARRAY32(sector.data, offset) & 0x00FF'FFFFu; // High byte is address.
                            offset += 4;
                        }
                }
                else // simply copy the first 128 colors.
                {
                    for(int i = 0, j = 0; j < 128; j++)
                    {
                        CLUT[j]  = as<uint32_t>(sector.data[i++]) << 16;
                        CLUT[j] |= as<uint32_t>(sector.data[i++]) << 8;
                        CLUT[j] |= sector.data[i++];
                    }
                }

                return;
            }
            else if((sector.subheader.submode & cdiv) == 0)
                return;

            // Green Book V.6.3.1
            const bool ascf = bit<7>(sector.subheader.codingInformation);
    //         const bool eolf = bit<6>(sector.subheader.codingInformation);
            const uint8_t resolution = bits<4, 5>(sector.subheader.codingInformation);
            const uint8_t coding = bits<0, 3>(sector.subheader.codingInformation);

            if(ascf || resolution == 2 || coding > 7) // Ignore invalid values.
                return;

            VideoInfo& v = video[sector.subheader.channelNumber];

            v.resolution = resolution;
            v.coding = coding;

            v.data.insert(v.data.end(), sector.data.begin(), sector.data.end());
        });

        const std::string dir = path + std::string(fileDir);
        std::filesystem::create_directories(dir);

        uint8_t pixels[768 * 560 * 4] = {0};
        uint8_t channel = 0;
        uint16_t y = 0;
        for(VideoInfo& v : video)
        {
            size_t index = 0;
            const uint16_t width = v.resolution == 0 ? 384 : 768;
            const uint16_t height = v.resolution == 3 ? 480 : 242;

            if(v.coding == 3 || v.coding == 4)
            {
                while(index < v.data.size())
                {
                    index += Video::decodeRunLengthLine(&pixels[width * 4 * y], &v.data[index], width, CLUT.data(), v.coding & 0x3);
                    y++;
                    if(y >= height)
                    {
                        uint8_t* pix = new uint8_t[width * height * 3];
                        Video::splitARGB(pixels, width * height * 4, nullptr, pix);
                        wxImage(width, height, pix, true).SaveFile(dir + file.name + "_" + std::to_string(channel) + "_" + std::to_string(v.record++) + ".bmp", wxBITMAP_TYPE_BMP);
                        delete[] pix;
                        y = 0;
                    }
                }
            }
            else
            {
                while(index < v.data.size())
                {
                    index += Video::decodeBitmapLine(&pixels[width * 4 * y], nullptr, &v.data[index], width, CLUT.data(), 0x00108080, codingLookUp[v.coding]);
                    y++;
                    if(y >= height)
                    {
                        uint8_t* pix = new uint8_t[width * height * 3];
                        Video::splitARGB(pixels, width * height * 4, nullptr, pix);
                        wxImage(width, height, pix, true).SaveFile(dir + file.name + "_" + std::to_string(channel) + "_" + std::to_string(v.record++) + ".bmp", wxBITMAP_TYPE_BMP);
                        delete[] pix;
                        y = 0;
                    }
                }
            }

            if(y > 0)
            {
                uint8_t* pix = new uint8_t[width * height * 4];
                Video::splitARGB(pixels, width * height * 4, nullptr, pix);
                wxImage(width, height, pix, true).SaveFile(dir + file.name + "_" + std::to_string(channel) + "_" + std::to_string(v.record++) + ".bmp", wxBITMAP_TYPE_BMP);
                delete[] pix;
                y = 0;
            }

            channel++;
        }
    });
}
