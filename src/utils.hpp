#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <string>

uint8_t convertPBCD(uint8_t data);
int32_t signExtend8(const int8_t data);
int32_t signExtend16(const int16_t data);
std::string toBinString(uint32_t value, uint8_t lengthInBits);

#endif // UTILS_HPP
