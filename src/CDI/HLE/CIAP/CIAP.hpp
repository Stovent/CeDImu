#ifndef CDI_HLE_CIAP_CIAP_HPP
#define CDI_HLE_CIAP_CIAP_HPP

class CDI;
#include "../../common/types.hpp"

#include <array>
#include <cstdint>

namespace HLE
{

class CIAP
{
public:
    CDI& cdi;

    CIAP() = delete;
    explicit CIAP(CDI& idc);

    void IncrementTime(double ns);

    uint16_t GetWord(uint32_t addr, BusFlags flags = BUS_NORMAL);

    void SetWord(uint32_t addr, uint16_t data, BusFlags flags = BUS_NORMAL);

private:
    std::array<uint16_t, 0x2600 / 2> registers;

    enum Registers
    {
        IER      = 0x2584,
        ISR_221  = 0x2586,
        TACS     = 0x2588,
        AACS     = 0x258A,
        TCM1     = 0x258C,
        ACM1     = 0x258E,
        ACM2     = 0x2590,
        FILE_221 = 0x2592,
        BMAN     = 0x2594,
        CCR      = 0x2596,
        A_SHDW   = 0x259A,
        AP_LEFT  = 0x25A0,
        AP_RIGHT = 0x25A2,
        AP_VOL   = 0x25A4,
        APCR     = 0x25A6,
        ACONF    = 0x25A8,
        ASTAT    = 0x25AA,
        ICR_221  = 0x25C0,
        DMACTL   = 0x25C2,
        ID       = 0x25C4,
        DLOAD    = 0x25FE,
    };
};

} // namespace HLE

#endif // CDI_HLE_CIAP_CIAP_HPP
