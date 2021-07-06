#ifndef CDI_CORES_IRTC_HPP
#define CDI_CORES_IRTC_HPP

#include <cstdint>

class CDI;

class IRTC
{
public:
    CDI& cdi;

    IRTC(CDI& idc) : cdi(idc) {}

    virtual void IncrementClock(const size_t ns) = 0;

    virtual uint8_t GetByte(const uint16_t addr) const = 0;
    virtual void SetByte(const uint16_t addr, const uint8_t data) = 0;
};

#endif // CDI_CORES_IRTC_HPP
