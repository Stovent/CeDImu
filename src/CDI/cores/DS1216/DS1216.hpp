#ifndef CDI_CORES_DS1216_DS1216_HPP
#define CDI_CORES_DS1216_DS1216_HPP

#include "../IRTC.hpp"

#include <array>

class DS1216 : public IRTC
{
    std::array<uint8_t, 8> clock;
    std::array<uint8_t, 0x8000> sram; // 32KB

public:
    DS1216() = delete;
    DS1216(CDI& idc);

    void IncrementClock(const size_t ns) override;

    uint8_t GetByte(const uint16_t addr) const override;
    void SetByte(const uint16_t addr, const uint8_t data) override;
};

#endif // CDI_CORES_DS1216_DS1216_HPP
