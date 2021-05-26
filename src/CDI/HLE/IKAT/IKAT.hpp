#ifndef HLE_IKAT_HPP
#define HLE_IKAT_HPP

#include "../../cores/ISlave.hpp"

#include <array>
#include <vector>

namespace HLE
{

enum Ports
{
    PA = 0,
    PB,
    PC,
    PD,
};

enum IKATRegisters
{
    PAWR = 0,
    PBWR,
    PCWR,
    PDWR,
    PARD,
    PBRD,
    PCRD,
    PDRD,
    PASR,
    PBSR,
    PCSR,
    PDSR,
    ISR,
    ICR,
    YCR,
};

class IKAT : public ISlave
{
    uint8_t registers[15];

    void ProcessCommandC(uint8_t data);
    void ProcessCommandD(uint8_t data);

    std::vector<uint8_t> commands[4];
    std::array<uint8_t, 4>::const_iterator responsesIterator[4];
    std::array<uint8_t, 4>::const_iterator responsesEnd[4];

    // https://github.com/cdifan/cdichips/blob/master/mc6805ikat.md
    std::array<uint8_t, 4> responseB4X = {0x43, 0x3F, 0, 0}; // Relative pointer state

    std::array<uint8_t, 3> responseCF4 = {0xA5, 0xF4, 0}; // Boot mode: 1 for service shell, 0 for player shell
    std::array<uint8_t, 4> responseCF6 = {0xA5, 0xF6, 1, 0xFF}; // Video standard: 1 for NTSC, 2 for PAL

    std::array<uint8_t, 3> responseDB0 = {0xB0, 0x02, 0x10}; // Disc status: 0x0210 according to cdiemu
    std::array<uint8_t, 4> responseDB1 = {0xB1, 0, 2, 0}; // Disc base: 00:02:00
    std::array<uint8_t, 4> responseDB2 = {0xB2, 0x20, 0, 0x10}; // Disc select: 0x200010 according to cdiemu

public:
    IKAT() = delete;
    IKAT(SCC68070& cpu, const bool PAL);

    virtual void UpdatePointerState() override;
    virtual void IncrementTime(const size_t ns) override;

    virtual uint8_t GetByte(const uint8_t addr) override;
    virtual void SetByte(const uint8_t addr, const uint8_t data) override;
};

} // namespace HLE

#endif // HLE_IKAT_HPP
