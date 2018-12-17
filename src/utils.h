#ifndef UTILS_HPP
#define UTILS_HPP

#include <stdint.h>

uint8_t convertPBCD(uint8_t data);
int32_t signExtend8(const int8_t data);
int32_t signExtend16(const int16_t data);

#endif // UTILS_HPP
