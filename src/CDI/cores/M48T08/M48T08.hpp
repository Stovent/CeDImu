#ifndef CDI_CORES_M48T08_M48T08_HPP
#define CDI_CORES_M48T08_M48T08_HPP

#include "../IRTC.hpp"

#include <array>
#include <cstdint>
#include <ctime>

class M48T08 : public IRTC
{
public:
    M48T08() = delete;
    M48T08(M48T08&) = delete;
    explicit M48T08(CDI& idc, std::time_t initialTime = 0, const uint8_t* state = nullptr);
    ~M48T08();

    void IncrementClock(const double ns) override;

    uint8_t GetByte(const uint16_t addr) override;
    void SetByte(const uint16_t addr, const uint8_t data) override;

private:
    Clock internalClock;
    std::array<uint8_t, 0x2000> sram;

    void ClockToSRAM();
    void SRAMToClock();
};

#endif // CDI_CORES_M48T08_M48T08_HPP
