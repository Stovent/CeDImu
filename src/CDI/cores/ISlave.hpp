#ifndef CDI_CORES_ISLAVE_HPP
#define CDI_CORES_ISLAVE_HPP

class CDI;
#include "../PointingDevice.hpp"

class ISlave
{
public:
    CDI& cdi;
    PointingDevice pointingDevice;
    const uint32_t busBase;

    ISlave(CDI& idc, uint32_t busbase, PointingDevice::Class deviceClass) : cdi(idc), pointingDevice(*this, deviceClass), busBase(busbase) {}
    virtual ~ISlave() = default;

    ISlave(const ISlave&) = delete;
    ISlave& operator=(const ISlave&) = delete;

    ISlave(ISlave&&) = delete;
    ISlave& operator=(ISlave&&) = delete;

    virtual void UpdatePointerState() = 0;
    virtual void IncrementTime(const size_t ns) = 0;

    virtual uint8_t GetByte(const uint8_t addr) = 0;
    virtual void SetByte(const uint8_t addr, const uint8_t data) = 0;
};

#endif // CDI_CORES_ISLAVE_HPP
