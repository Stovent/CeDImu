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
void writeWAV(std::ofstream& out, const Audio::WAVHeader& wavHeader, const std::vector<int16_t>& left, const std::vector<int16_t>& right);

} // namespace Audio

#endif // CDI_COMMON_AUDIO_HPP
