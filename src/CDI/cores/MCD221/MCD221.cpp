#include "MCD221.hpp"
#include "../../CDI.hpp"
#include "../../common/Callbacks.hpp"

enum MCD221Registers
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

MCD221::MCD221(CDI& idc) :
    cdi(idc),
    registers{0}
{
}

void MCD221::IncrementTime(const double ns)
{
}

uint16_t MCD221::GetWord(const uint32_t addr, const uint8_t flags)
{
    const uint16_t data = addr == ID ? 0xCD02 : registers[addr >> 1];

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::CDIC, "Get", "Word", cdi.board->cpu.currentPC, addr, data}); })

    return data;
}

void MCD221::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr < 0x2600)
        registers[addr >> 1] = data;

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::CDIC, "Set", "Word", cdi.board->cpu.currentPC, addr, data}); })
}
