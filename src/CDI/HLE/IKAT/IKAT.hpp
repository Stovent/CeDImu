#ifndef CDI_HLE_IKAT_HPP
#define CDI_HLE_IKAT_HPP

#include "../../cores/ISlave.hpp"

#include <array>
#include <deque>
#include <string>
#include <vector>

namespace HLE
{

class IKAT : public ISlave
{
public:
    IKAT(CDI& idc, bool PAL, uint32_t busbase, PointingDevice::Class deviceClass);

    IKAT(const IKAT&) = delete;
    IKAT& operator=(const IKAT&) = delete;

    IKAT(IKAT&&) = delete;
    IKAT& operator=(IKAT&&) = delete;

    virtual void UpdatePointerState() override;
    virtual void IncrementTime(size_t ns) override;

    virtual uint8_t PeekByte(uint8_t addr) const noexcept override;

    /** \brief Returns the value of the given host register.
     * \param addr The address of the register (from the MCU side).
     */
    virtual uint8_t GetByte(uint8_t addr, BusFlags flags) override;

    /** \brief Sets the host register to the given value.
     * \param addr The address of the register (from the MCU side).
     */
    virtual void SetByte(uint8_t addr, uint8_t data, BusFlags flags) override;

private:
    enum Channels
    {
        CHA = 0,
        CHB,
        CHC,
        CHD,
    };

    enum Registers
    {
        CHA_IN = 0,
        CHB_IN,
        CHC_IN,
        CHD_IN,

        CHA_OUT,
        CHB_OUT,
        CHC_OUT,
        CHD_OUT,

        CHA_SR,
        CHB_SR,
        CHC_SR,
        CHD_SR,

        ISR,
        IMR,
        Mode,
        _size,
    };

    std::deque<uint8_t> channelIn[4]; // MCU IN, host OUT
    std::deque<uint8_t> channelOut[4]; // MCU OUT, host IN
    std::array<uint8_t, Registers::_size> registers;

    void ProcessCommandC();
    void ProcessCommandD();

    // https://github.com/cdifan/cdichips/blob/master/mc6805ikat.md
    // For F0, F7 and F8, the low level test reads 4 bytes but the last one is unused.
    static constexpr std::array<uint8_t, 3> responseCF0{0xA5, 0xF0, 0x7F}; // Release Number IKAT. Used by the copyright screen as the last 2 digits of the center block.
    static constexpr std::array<uint8_t, 3> responseCF4{0xA5, 0xF4, 0}; // Boot mode: 1 for service shell, 0 for player shell.
    std::array<uint8_t, 4> responseCF6{0xA5, 0xF6, 1, 0xFF}; // Video standard: 1 for NTSC, 2 for PAL.
    static constexpr std::array<uint8_t, 3> responseCF7{0xA5, 0xF7, 0}; // CD60 low level test. Must be 0 or test fails.
    static constexpr std::array<uint8_t, 3> responseCF8{0xA5, 0xF8, 0}; // DSIC2 low level test. Must be 0 or test fails.

    static constexpr std::array<uint8_t, 4> responseDB0{0xB0, 0x02, 0x10, 0}; // Disc status: 0x0210 according to cdiemu.
    static constexpr std::array<uint8_t, 4> responseDB1{0xB1, 0, 2, 0}; // Disc base: 00:02:00.
    static constexpr std::array<uint8_t, 4> responseDB2{0xB2, 0x20, 0, 0x10}; // Disc select: 0x200010 according to cdiemu.

    const std::array<uint8_t, 4>* delayedRsp[4];
    uint32_t delayedRspFrame[4];

    static constexpr int INT_MASK[4] = {0x02, 0x08, 0x20, 0x80};

    static std::string getPortName(uint8_t index);
};

} // namespace HLE

#endif // CDI_HLE_IKAT_HPP
