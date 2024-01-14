#include "Audio.hpp"
#include "utils.hpp"

namespace Audio
{

static constexpr int K0[4] = {0, 240, 460, 392};
static constexpr int K1[4] = {0, 0, -208, -220};

static int lk0 = 0;
static int rk0 = 0;
static int lk1 = 0;
static int rk1 = 0;

/** \brief Reset to 0 the delay used by the filters k0 and k1.
 */
void resetAudioFiltersDelay()
{
    lk0 = 0;
    rk0 = 0;
    lk1 = 0;
    rk1 = 0;
}

/** \brief Implements the ADPCM decoder.
 * \tparam SU The number of sound units (4 for level A, 8 for level BC).
 * \tparam GAIN Base exponent for the gain (8 for level A, 12 for level BC).
 * \param sd The sound datas.
 * \param ranges The ranges.
 * \param filters The filters.
 * \param stereo true for stereo, false for mono.
 * \param left Where the decoded samples for left channel will be written (used in mono).
 * \param right Where the decoded samples for right channel will be written (unused in mono).
 * \return The number of samples decoded (should be 112 for level A and 224 for level BC).
 */
template<int SU, uint8_t GAIN>
static uint8_t decodeADPCM(int8_t sd[][28], uint8_t* ranges, uint8_t* filters, bool stereo, std::vector<int16_t>& left, std::vector<int16_t>& right)
{
    for(int su = 0; su < SU; su++)
    {
        const uint16_t gain = 2 << (GAIN - ranges[su]);
        for(uint8_t ss = 0; ss < 28; ss++)
        {
            if(stereo && su & 1)
            {
                const int16_t sample = lims16((sd[su][ss] * gain) + ((rk0*K0[filters[su]] + rk1*K1[filters[su]]) / 256));
                rk1 = rk0;
                rk0 = sample;
                right.push_back(sample);
            }
            else
            {
                const int16_t sample = lims16((sd[su][ss] * gain) + ((lk0*K0[filters[su]] + lk1*K1[filters[su]]) / 256));
                lk1 = lk0;
                lk0 = sample;
                left.push_back(sample);
            }
        }
    }

    return 28 * SU;
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
 *
 * Special thanks to this thread (http://www.cdinteractive.co.uk/forums/cdinteractive/viewtopic.php?t=3191)
 * for making me understand how the k0 and k1 filters worked in ADCPM decoder
 */
uint8_t decodeLevelASoundGroup(const bool stereo, const uint8_t data[128], std::vector<int16_t>& left, std::vector<int16_t>& right)
{
    uint8_t range[4];
    uint8_t filter[4];
    for(uint8_t i = 0; i < 4; i++)
    {
        range[i] = bits<0, 3>(data[i]);
        filter[i] = bits<4, 7>(data[i]);
    }

    uint8_t index = 16;
    int8_t SD[4][28]; // sound data
    for(uint8_t ss = 0; ss < 28; ss++) // sound sample
        for(uint8_t su = 0; su < 4; su++) // sound unit
            SD[su][ss] = data[index++];

    return decodeADPCM<4, 8>(SD, range, filter, stereo, left, right);
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
    uint8_t range[8];
    uint8_t filter[8];
    for(uint16_t i = 0; i < 8; i++)
    {
        range[i] = bits<0, 3>(data[i + 4]);
        filter[i] = bits<4, 7>(data[i + 4]);
    }

    uint8_t index = 16;
    int8_t SD[8][28]; // sound data
    for(uint8_t ss = 0; ss < 28; ss++) // sound sample
        for(uint8_t su = 0; su < 8;) // sound unit
        {
            const uint8_t SB = data[index++];

            int8_t SD0 = bits<0, 3>(SB);
            if(SD0 >= 8)
                SD0 -= 16;

            int8_t SD1 = bits<4, 7>(SB);
            if(SD1 >= 8)
                SD1 -= 16;

            SD[su++][ss] = SD0;
            SD[su++][ss] = SD1;
        }

    return decodeADPCM<8, 12>(SD, range, filter, stereo, left, right);
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
