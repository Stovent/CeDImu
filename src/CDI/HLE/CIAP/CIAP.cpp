#include "CIAP.hpp"
#include "../../CDI.hpp"
#include "../../common/Callbacks.hpp"

namespace HLE
{

CIAP::CIAP(CDI& idc)
    : cdi(idc)
    , registers{0}
{
    registers[ID >> 1] = 0xCD02;
}

void CIAP::IncrementTime(const double ns)
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

} // namespace HLE
