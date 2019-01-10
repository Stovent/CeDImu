#include "utils.hpp"

uint8_t convertPBCD(uint8_t data)
{
    return ((data & 0xF0) >> 4) * 10 + (data & 0x0F);
}

int32_t signExtend8(const int8_t data)
{
    return data;
}

int32_t signExtend16(const int16_t data)
{
    return data;
}

std::string toBinString(uint32_t value, uint8_t lengthInBits)
{
    std::string tmp;
    for(int i = 0; i < lengthInBits; i++)
        if(value & (1 << (7-i)))
            tmp += "1";
        else
            tmp +="0";
    return tmp;
}
