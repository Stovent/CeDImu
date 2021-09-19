#ifndef CDI_CORES_ISLAVE_HPP
#define CDI_CORES_ISLAVE_HPP

class CDI;
#include "../devices/PointingDevice.hpp"
#include "../devices/ManeuveringDevice.hpp"

#include <memory>

class ISlave
{
public:
    CDI& cdi;
    std::unique_ptr<PointingDevice> pointingDevice;
    const uint32_t busBase;

    ISlave() = delete;
    ISlave(CDI& idc, uint32_t busbase) : cdi(idc), pointingDevice(std::make_unique<ManeuveringDevice>(*this)), busBase(busbase) {}
    virtual ~ISlave() {}

    virtual void UpdatePointerState() = 0;
    virtual void IncrementTime(const size_t ns) = 0;

    virtual uint8_t GetByte(const uint8_t addr) = 0;
    virtual void SetByte(const uint8_t addr, const uint8_t data) = 0;
};

#endif // CDI_CORES_ISLAVE_HPP
