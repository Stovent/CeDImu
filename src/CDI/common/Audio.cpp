#include "Audio.hpp"
#include "utils.hpp"

#include <cmath>

namespace Audio
{

static constexpr float k0[4] = {0.0, 0.9375, 1.796875, 1.53125};
static constexpr float k1[4] = {0.0, 0.0, -0.8125, -0.859375};

static int32_t lk0 = 0;
static int32_t rk0 = 0;
static int32_t lk1 = 0;
static int32_t rk1 = 0;

/** \brief Reset to 0 the delay used by the filters k0 and k1.
 */
void resetAudioFiltersDelay()
{
    lk0 = 0;
    rk0 = 0;
    lk1 = 0;
    rk1 = 0;
}

/** \brief Decode a raw audio sector into 16-bit PCM.
 *
 * \param  levelA True if input data is encoded in Level A audio, false if Level B or C.
 * \param  stereo True if input data is stereo, false if mono.
 * \param  data Raw input data from the disc.
 * \param  left Destination left audio channel. If audio is mono, it will contain the decoded data and right will remain untouched.
 * \param  right Destination right audio channel. If audio is mono, NULL can be passed safely.
 * \return number of samples decoded (should always be 4032).
 */
uint16_t decodeAudioSector(const bool levelA, const bool stereo, const uint8_t data[2304], std::vector<int16_t>& left, std::vector<int16_t>& right)
{
    uint16_t index = 0;
    if(levelA) // Level A (8 bits per sample)
    {
        for(uint8_t sg = 0; sg < 18; sg++)
        {
            index += decodeLevelASoundGroup(stereo, &data[128 * sg], left, right);
        }
    }
    else // Level B and C (4 bits per sample)
    {
        for(uint8_t sg = 0; sg < 18; sg++)
        {
            index += decodeLevelBCSoundGroup(stereo, &data[128 * sg], left, right);
        }
    }
    return index;
}

/** \brief Decode a Level A sound group.
 *
 * \param  stereo True if input data is stereo, false if mono.
 * \param  data Raw sound group.
 * \param  left Destination left audio channel. If audio is mono, it will contain the decoded data and right will remain untouched.
 * \param  right Destination right audio channel. If audio is mono, it will remain untouched.
 * \return number of samples decoded.
 */
uint8_t decodeLevelASoundGroup(const bool stereo, const uint8_t data[128], std::vector<int16_t>& left, std::vector<int16_t>& right)
{
    uint8_t index = 16;
    uint8_t range[4];
    uint8_t filter[4];
     int8_t SD[4][28]; // sound data

    for(uint8_t i = 0; i < 4; i++)
    {
        range[i] = data[i] & 0x0F;
        filter[i] = data[i] >> 4;
    }

    for(uint8_t ss = 0; ss < 28; ss += 2) // sound sample
        for(uint8_t su = 0; su < 4; su++) // sound unit
            SD[su][ss] = data[index++];

    // ADPCM decoder
    index = 0;
    for(uint16_t su = 0; su < 4; su++)
    {
        uint16_t gain = pow(2, 8 - range[su]);
        if(stereo)
        {
            for(uint8_t ss = 0; ss < 28; ss++)
            {
                if(su & 1)
                {
                    int32_t data = (SD[su][ss] * gain) + (rk0*k0[filter[su]] + rk1*k1[filter[su]]);
                    rk1 = rk0;
                    rk0 = data;
                    right.push_back(lim16(data));
                    index++;
                }
                else
                {
                    int32_t data = (SD[su][ss] * gain) + (lk0*k0[filter[su]] + lk1*k1[filter[su]]);
                    lk1 = lk0;
                    lk0 = data;
                    left.push_back(lim16(data));
                    index++;
                }
            }
        }
        else
        {
            for(uint8_t ss = 0; ss < 28; ss++)
            {
                int32_t data = (SD[su][ss] * gain) + (lk0*k0[filter[su]] + lk1*k1[filter[su]]);
                lk1 = lk0;
                lk0 = data;
                left.push_back(lim16(data));
                // in case of a mono sector between 2 stereo, fill to avoid channel data size mismatch
                if(right.size())
                    right.push_back(0);
                index++;
            }
        }
    }
    return index;
}

/** \brief Decode a Level B or C sound group.
 *
 * \param  stereo True if input data is stereo, false if mono.
 * \param  data Raw sound group.
 * \param  left Destination left audio channel. If audio is mono, it will contain the decoded data and right will remain untouched.
 * \param  right Destination right audio channel. If audio is mono, it will remain untouched.
 * \return number of samples decoded.
 */
uint8_t decodeLevelBCSoundGroup(const bool stereo, const uint8_t data[128], std::vector<int16_t>& left, std::vector<int16_t>& right)
{
    uint8_t index = 4;
    uint8_t range[8];
    uint8_t filter[8];
     int8_t SD[8][28]; // sound data

    for(uint16_t i = 0; i < 8; i++)
    {
        range[i] = data[i + index] & 0x0F;
        filter[i] = data[i + index] >> 4;
    }

    index = 16;
    for(uint8_t ss = 0; ss < 28; ss++) // sound sample
        for(uint8_t su = 0; su < 8; su += 2) // sound unit
        {
            const uint8_t SB = data[index++];
            SD[su][ss] = SB & 0x0F;
            if(SD[su][ss] >= 8) SD[su][ss] -= 16;
            SD[su+1][ss] = SB >> 4;
            if(SD[su+1][ss] >= 8) SD[su+1][ss] -= 16;
        }

    // ADPCM decoder
    index = 0;
    for(int su = 0; su < 8; su++)
    {
        uint16_t gain = pow(2, 12 - range[su]);
        if(stereo)
        {
            for(uint8_t ss = 0; ss < 28; ss++)
            {
                if(su & 1)
                {
                    int32_t data = (SD[su][ss] * gain) + (rk0*k0[filter[su]] + rk1*k1[filter[su]]);
                    rk1 = rk0;
                    rk0 = data;
                    right.push_back(lim16(data));
                    index++;
                }
                else
                {
                    int32_t data = (SD[su][ss] * gain) + (lk0*k0[filter[su]] + lk1*k1[filter[su]]);
                    lk1 = lk0;
                    lk0 = data;
                    left.push_back(lim16(data));
                    index++;
                }
            }
        }
        else
        {
            for(uint8_t ss = 0; ss < 28; ss++)
            {
                int32_t data = (SD[su][ss] * gain) + (lk0*k0[filter[su]] + lk1*k1[filter[su]]);
                lk1 = lk0;
                lk0 = data;
                left.push_back(lim16(data));
                // in case of a mono sector between 2 stereo, fill to avoid channel data size mismatch
                if(right.size())
                    right.push_back(0);
                index++;
            }
        }
    }
    return index;
}

/** \brief Writes the audio data (16 bit signed PCM) in the given file.
 *
 * \param  out The output where the data will be written to.
 * \param  wavHeader a struct that holds information on the audio data.
 * \param  left The left audio channel.
 * \param  right The right audio channel.
 *
 * Writes the WAV header in the file using {wavHeader}, and then writes the audio data.
 * The file is written in little endian format.
 * If {right} is empty, writes only the content of {left} (mono file).
 * If {right} and {left} have different size, writes only the
 * first min(left.size(), right.size()) samples from both audio channels.
 */
void writeWAV(std::ofstream& out, const Audio::WAVHeader& wavHeader, const std::vector<int16_t>& left, const std::vector<int16_t>& right)
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

} // namespace Audio
