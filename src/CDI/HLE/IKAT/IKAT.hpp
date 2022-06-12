#ifndef CDI_HLE_IKAT_HPP
#define CDI_HLE_IKAT_HPP

#include "../../cores/ISlave.hpp"

#include <array>
#include <deque>
#include <vector>

namespace HLE
{

class IKAT : public ISlave
{
public:
    IKAT() = delete;
    IKAT(const IKAT&) = delete;
    IKAT(CDI& idc, bool PAL, uint32_t busbase, PointingDevice::Type deviceType);
    IKAT& operator=(const IKAT&) = delete;

    virtual void UpdatePointerState() override;
    virtual void IncrementTime(const Cycles& c) override;

    virtual uint8_t GetByte(const uint8_t addr) override;
    virtual void SetByte(const uint8_t addr, const uint8_t data) override;

private:
    std::deque<uint8_t> channelIn[4]; // MCU IN, host OUT
    std::deque<uint8_t> channelOut[4]; // MCU OUT, host IN
    uint8_t registers[15];

    void ProcessCommandC();
    void ProcessCommandD();

    // https://github.com/cdifan/cdichips/blob/master/mc6805ikat.md
    std::array<uint8_t, 3> responseCF4 = {0xA5, 0xF4, 0}; // Boot mode: 1 for service shell, 0 for player shell
    std::array<uint8_t, 4> responseCF6 = {0xA5, 0xF6, 1, 0xFF}; // Video standard: 1 for NTSC, 2 for PAL

    std::array<uint8_t, 4> responseDB0 = {0xB0, 0x02, 0x10, 0}; // Disc status: 0x0210 according to cdiemu
    std::array<uint8_t, 4> responseDB1 = {0xB1, 0, 2, 0}; // Disc base: 00:02:00
    std::array<uint8_t, 4> responseDB2 = {0xB2, 0x20, 0, 0x10}; // Disc select: 0x200010 according to cdiemu

    std::array<uint8_t, 4>* delayedRsp[4];
    uint32_t delayedRspFrame[4];
};

} // namespace HLE

#endif // CDI_HLE_IKAT_HPP
