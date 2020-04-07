#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <cstdint>
#include <vector>

void resetAudioFiltersDelay();
uint16_t decodeAudioSector(const bool levelA, const bool stereo, const uint8_t data[2304], std::vector<int16_t>& left, std::vector<int16_t>& right);
uint8_t decodeLevelASoundGroup(const bool stereo, const uint8_t data[128], std::vector<int16_t>& left, std::vector<int16_t>& right);
uint8_t decodeLevelBCSoundGroup(const bool stereo, const uint8_t data[128], std::vector<int16_t>& left, std::vector<int16_t>& right);

#endif // AUDIO_HPP
