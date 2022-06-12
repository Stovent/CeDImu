#ifndef CDI_CORES_IRTC_HPP
#define CDI_CORES_IRTC_HPP

class CDI;
#include "../common/Cycles.hpp"

#include <cstdint>
#include <ctime>

class IRTC
{
public:
    static constexpr std::time_t defaultTime = 599616000; // 1989/01/01 00:00:00
    CDI& cdi;

    IRTC() = delete;
    IRTC(IRTC&) = delete;
    explicit IRTC(CDI& idc) : cdi(idc) {}
    virtual ~IRTC() {}

    virtual void IncrementClock(const Cycles& c) = 0;

    virtual uint8_t GetByte(const uint16_t addr) = 0;
    virtual void SetByte(const uint16_t addr, const uint8_t data) = 0;
};

#endif // CDI_CORES_IRTC_HPP
