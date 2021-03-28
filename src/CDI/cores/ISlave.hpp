#ifndef ISLAVE_HPP
#define ISLAVE_HPP

#include <cstdint>

class ISlave
{
public:
    virtual ~ISlave() {}

    virtual uint8_t GetByte(const uint8_t addr) = 0;
    virtual void SetByte(const uint8_t addr, const uint8_t data) = 0;
};

#endif // ISLAVE_HPP
