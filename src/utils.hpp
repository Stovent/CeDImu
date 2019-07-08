#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <string>

uint8_t convertPBCD(uint8_t data);
int32_t signExtend8(const int8_t data);
int16_t signExtend816(const int8_t data);
int32_t signExtend16(const int16_t data);
bool isEven(int number);
std::string toHex(uint32_t number);
std::string toBinString(uint32_t value, uint8_t lengthInBits);
uint32_t binStringToInt(std::string s);

#endif // UTILS_HPP
