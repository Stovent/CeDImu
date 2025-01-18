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

void CIAP::IncrementTime(const double)
{
    registers[ISR_221 >> 1] = 9; // Data interrupt.
}

uint16_t CIAP::GetWord(const uint32_t addr, const BusFlags flags)
{
    const uint16_t data = registers[addr >> 1];

    if(addr == ISR_221)
        registers[ISR_221 >> 1] = 0; // Clear ISR bits on read.

    LOG(if(flags.log) { if(cdi.m_callbacks.HasOnLogMemoryAccess()) \
            cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::CDIC, "Get", "Word", cdi.m_cpu.currentPC, addr, data}); })

    return data;
}

void CIAP::SetWord(const uint32_t addr, const uint16_t data, const BusFlags flags)
{
    if(addr < 0x2600)
        registers[addr >> 1] = data;

    LOG(if(flags.log) { if(cdi.m_callbacks.HasOnLogMemoryAccess()) \
            cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::CDIC, "Set", "Word", cdi.m_cpu.currentPC, addr, data}); })
}

} // namespace HLE
