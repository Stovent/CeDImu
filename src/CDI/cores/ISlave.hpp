#ifndef ISLAVE_HPP
#define ISLAVE_HPP

#include <cstdint>

class ISlave
{
public:
    virtual uint8_t GetByte(const uint8_t addr);
    virtual void SetByte(const uint8_t addr, const uint8_t data);
};

#endif // ISLAVE_HPP
