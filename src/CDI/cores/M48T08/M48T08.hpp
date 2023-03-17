#ifndef CDI_CORES_M48T08_M48T08_HPP
#define CDI_CORES_M48T08_M48T08_HPP

#include "../IRTC.hpp"

#include <array>
#include <cstdint>
#include <ctime>
#include <span>

class M48T08 : public IRTC
{
public:
    M48T08() = delete;
    M48T08(M48T08&) = delete;
    explicit M48T08(CDI& cdi, std::span<const uint8_t> state, std::time_t initialTime = 0);
    ~M48T08();

    void IncrementClock(const double ns) override;

    uint8_t GetByte(const uint16_t addr) override;
    void SetByte(const uint16_t addr, const uint8_t data) override;

private:
    std::array<uint8_t, 0x2000> sram;
    Clock internalClock;

    void ClockToSRAM();
    void SRAMToClock();
};

#endif // CDI_CORES_M48T08_M48T08_HPP
