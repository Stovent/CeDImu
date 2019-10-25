#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <sstream>
#include <string>

inline uint8_t convertPBCD(const uint8_t data)
{
    return (data >> 4) * 10 + (data & 0x0F);
}

inline int32_t signExtend8(const int8_t data)
{
    return data;
}

inline int16_t signExtend816(const int8_t data)
{
    return data;
}

inline int32_t signExtend16(const int16_t data)
{
    return data;
}

inline bool isEven(const int number)
{
    if(number & 1)
        return false;
    else
        return true;
}

inline std::string toHex(const uint32_t number)
{
    std::stringstream ss;
    ss << std::hex << number;
    return ss.str();
}

inline std::string toBinString(uint32_t value, uint8_t lengthInBits)
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

inline uint32_t binStringToInt(std::string s)
{
    uint32_t ret = 0;
    uint32_t base = 1 << (s.length()-1);
    for(uint8_t i = 0; i < s.length(); i++)
        if(s[i] == '1')
            ret |= base >> i;
    return ret;
}

#endif // UTILS_HPP
