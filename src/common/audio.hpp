#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <cstdint>
#include <fstream>
#include <vector>

namespace Audio
{

enum CodingInformation
{
    emphasis = 0b01000000,
    bps      = 0b00110000, // bits per sample
    sf       = 0b00001100, // sampling frequency
    ms       = 0b00000011  // mono/stereo
};

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

#endif // AUDIO_HPP
