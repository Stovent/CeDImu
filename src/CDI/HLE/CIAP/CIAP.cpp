#include "CIAP.hpp"
#include "../../CDI.hpp"
#include "../../common/Callbacks.hpp"

enum CIAPRegisters
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

CIAP::CIAP(CDI& idc)
    : cdi(idc)
    , registers{0}
{
    registers[ID >> 1] = 0xCD02;
}

void CIAP::IncrementTime(const Cycles& c)
{
    registers[ISR_221 >> 1] = 9; // Data interrupt.
}

uint16_t CIAP::GetWord(const uint32_t addr, const uint8_t flags)
{
    const uint16_t data = registers[addr >> 1];

    if(addr == ISR_221)
        registers[ISR_221 >> 1] = 0; // Clear ISR bits on read.

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::CDIC, "Get", "Word", cdi.board->cpu.currentPC, addr, data}); })

    return data;
}

void CIAP::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr < 0x2600)
        registers[addr >> 1] = data;

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::CDIC, "Set", "Word", cdi.board->cpu.currentPC, addr, data}); })
}
