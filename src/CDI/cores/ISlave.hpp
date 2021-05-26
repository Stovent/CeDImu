#ifndef ISLAVE_HPP
#define ISLAVE_HPP

#include "SCC68070/SCC68070.hpp"
#include "../devices/PointingDevice.hpp"
#include "../devices/ManeuveringDevice.hpp"

#include <memory>

class ISlave
{
public:
    SCC68070& cpu;
    std::unique_ptr<PointingDevice> pointingDevice;

    ISlave() = delete;
    explicit ISlave(SCC68070& scc68070) : cpu(scc68070), pointingDevice(new ManeuveringDevice(*this)) {}
    virtual ~ISlave() {}

    virtual void UpdatePointerState() = 0;
    virtual void IncrementTime(const size_t ns) = 0;

    virtual uint8_t GetByte(const uint8_t addr) = 0;
    virtual void SetByte(const uint8_t addr, const uint8_t data) = 0;
};

#endif // ISLAVE_HPP
