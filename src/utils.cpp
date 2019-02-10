#include "utils.hpp"

uint8_t convertPBCD(uint8_t data)
{
    return (data >> 4) * 10 + (data & 0x0F);
}

int32_t signExtend8(const int8_t data)
{
    return data;
}

int16_t signExtend816(const int8_t data)
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
    uint32_t mask = 1 << (lengthInBits-1);
    while(lengthInBits)
    {
        if(value & mask)
            tmp += "1";
        else
            tmp += "0";
        value <<= 1;
        lengthInBits--;
    }
    return tmp;
}
