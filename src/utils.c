#include "utils.h"

uint8_t convertPBCD(uint8_t data)
{
    return ((value & 0xF0) >> 4) * 10 + (value & 0x0F);
}

int32_t signExtend8(const int8_t data)
{
    return data;
}

int32_t signExtend16(const int16_t data)
{
    return data;
}
