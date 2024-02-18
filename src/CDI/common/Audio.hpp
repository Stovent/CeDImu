#ifndef CDI_COMMON_AUDIO_HPP
#define CDI_COMMON_AUDIO_HPP

#include <cstdint>
#include <fstream>
#include <vector>

namespace Audio
{

struct WAVHeader
{
    uint16_t channelNumber;
    uint32_t frequency;
};

void resetAudioFiltersDelay();
uint16_t decodeAudioSector(const bool levelA, const bool stereo, const uint8_t data[2304], std::vector<int16_t>& left, std::vector<int16_t>& right);
uint8_t decodeLevelASoundGroup(const bool stereo, const uint8_t data[128], std::vector<int16_t>& left, std::vector<int16_t>& right);
uint8_t decodeLevelBCSoundGroup(const bool stereo, const uint8_t data[128], std::vector<int16_t>& left, std::vector<int16_t>& right);
void writeWAV(const std::string& basename, std::vector<int16_t>& left, std::vector<int16_t>& right, uint8_t channel, int record, uint8_t bps, uint8_t sf, uint8_t ms);

} // namespace Audio

#endif // CDI_COMMON_AUDIO_HPP
