#ifndef CDI_COMMON_AUDIO_HPP
#define CDI_COMMON_AUDIO_HPP

#include <cstdint>
#include <fstream>
#include <vector>

namespace Audio
{

/** \brief Stores the samples delayed by the ADPCM decoder. */
struct SamplesDelay
{
    int16_t lk0 = 0; /**< Left k0 delayed sample. */
    int16_t rk0 = 0; /**< right k0 delayed sample. */
    int16_t lk1 = 0; /**< Left k1 delayed sample. */
    int16_t rk1 = 0; /**< right k1 delayed sample. */
};

struct WAVHeader
{
    uint16_t channelNumber;
    uint32_t frequency;
};

uint16_t decodeAudioSector(SamplesDelay& delay, bool levelA, bool stereo, const uint8_t data[2304], std::vector<int16_t>& left, std::vector<int16_t>& right);
uint8_t decodeLevelASoundGroup(SamplesDelay& delay, bool stereo, const uint8_t data[128], std::vector<int16_t>& left, std::vector<int16_t>& right);
uint8_t decodeLevelBCSoundGroup(SamplesDelay& delay, bool stereo, const uint8_t data[128], std::vector<int16_t>& left, std::vector<int16_t>& right);
void writeWAV(const std::string& basename, std::vector<int16_t>& left, std::vector<int16_t>& right, uint8_t channel, int record, uint8_t bps, uint8_t sf, uint8_t ms);

} // namespace Audio

#endif // CDI_COMMON_AUDIO_HPP
