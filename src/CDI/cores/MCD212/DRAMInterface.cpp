#include "MCD212.hpp"
#include "../SCC68070/SCC68070.hpp"
#include "../../CDI.hpp"
#include "../../common/Callbacks.hpp"
#include "../../common/utils.hpp"

uint8_t MCD212::PeekByte(const uint32_t addr) const noexcept
{
    if(addr < 0x400000)
    {
        return m_memory.at(addr);
    }

    if(addr < 0x4FFC00)
    {
        return m_bios.At(addr - 0x400000);
    }

    if(addr == 0x4FFFE1)
    {
        return m_registerCSR2R;
    }

    if(addr == 0x4FFFF1)
    {
        return m_registerCSR1R;
    }

    std::terminate(); // TODO: is this the best way? or should I return 0 or an optional?
}

uint16_t MCD212::PeekWord(const uint32_t addr) const noexcept
{
    if(addr < 0x400000)
    {
        return getArray16(m_memory, addr);
    }

    if(addr < 0x4FFC00)
    {
        return getArray16(m_bios, addr - 0x400000);
    }

    if(addr == 0x4FFFE0) // word size: MSB is 0, LSB is the register
    {
        return m_registerCSR2R;
    }

    if(addr == 0x4FFFF0) // word size: MSB is 0, LSB is the register
    {
        return m_registerCSR1R;
    }

    std::terminate(); // TODO: is this the best way? or should I return 0 or an optional?
}

uint8_t MCD212::GetByte(const uint32_t addr, const BusFlags flags)
{
    uint8_t data;
    LOG(MemoryAccessLocation location = MemoryAccessLocation::OutOfRange;)

    if(addr < 0x400000)
    {
        data = m_memory[addr];
        LOG(location = MemoryAccessLocation::RAM;)
    }
    else if(addr < 0x4FFC00)
    {
        data = m_bios[addr - 0x400000];
        LOG(location = MemoryAccessLocation::BIOS;)
    }
    else if(addr == 0x4FFFE1)
    {
        data = m_registerCSR2R;
        m_registerCSR2R = 0; // clear IT1, IT2 and BE bits on status read
        LOG(location = MemoryAccessLocation::VDSC;)
    }
    else if(addr == 0x4FFFF1)
    {
        data = m_registerCSR1R;
        LOG(location = MemoryAccessLocation::VDSC;)
    }
    else
    {
        LOG(if(flags.log && m_cdi.m_callbacks.HasOnLogMemoryAccess()) \
                m_cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Get", "Byte", m_cdi.m_cpu.currentPC, addr, 0});)
        throw SCC68070::Exception(SCC68070::BusError);
    }

    LOG(if(flags.log && m_cdi.m_callbacks.HasOnLogMemoryAccess()) \
            m_cdi.m_callbacks.OnLogMemoryAccess({location, "Get", "Byte", m_cdi.m_cpu.currentPC, addr, data});)

    return data;
}

uint16_t MCD212::GetWord(const uint32_t addr, const BusFlags flags)
{
    if(m_memorySwapCount < 4) [[unlikely]]
    {
        m_memorySwapCount++;
        return getArray16(m_bios, addr);
    }

    uint16_t data;
    LOG(MemoryAccessLocation location = MemoryAccessLocation::OutOfRange;)

    if(addr < 0x400000)
    {
        data = getArray16(m_memory, addr);
        LOG(location = MemoryAccessLocation::RAM;)
    }
    else if(addr < 0x4FFC00)
    {
        data = getArray16(m_bios, addr - 0x400000);
        LOG(location = MemoryAccessLocation::BIOS;)
    }
    else if(addr == 0x4FFFE0) // word size: MSB is 0, LSB is the register
    {
        data = m_registerCSR2R;
        m_registerCSR2R = 0; // clear IT1, IT2 and BE bits on status read
        LOG(location = MemoryAccessLocation::VDSC;)
    }
    else if(addr == 0x4FFFF0) // word size: MSB is 0, LSB is the register
    {
        data = m_registerCSR1R;
        LOG(location = MemoryAccessLocation::VDSC;)
    }
    else
    {
        LOG(if(flags.log && m_cdi.m_callbacks.HasOnLogMemoryAccess()) \
                m_cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Get", "Word", m_cdi.m_cpu.currentPC, addr, 0});)
        throw SCC68070::Exception(SCC68070::BusError);
    }

    LOG(if(flags.log && m_cdi.m_callbacks.HasOnLogMemoryAccess()) \
            m_cdi.m_callbacks.OnLogMemoryAccess({location, "Get", "Word", m_cdi.m_cpu.currentPC, addr, data});)

    return data;
}

uint32_t MCD212::GetControlInstruction(const uint32_t addr)
{
    return as<uint32_t>(GetWord(addr, BUS_INSTRUCTION)) << 16 | GetWord(addr + 2, BUS_INSTRUCTION);
}

void MCD212::SetByte(const uint32_t addr, const uint8_t data, const BusFlags flags)
{
    if(addr < 0x400000)
    {
        m_memory[addr] = data;
        LOG(if(flags.log && m_cdi.m_callbacks.HasOnLogMemoryAccess()) \
                m_cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::RAM, "Set", "Byte", m_cdi.m_cpu.currentPC, addr, data});)
        return;
    }

    if(addr >= 0x4FFFE0 && addr < 0x500000)
    {
        if(isEven(addr))
        {
            m_internalRegisters[addr - 0x4FFFE0] &= 0x00FF;
            m_internalRegisters[addr - 0x4FFFE0] |= as<uint16_t>(data) << 8;
        }
        else
        {
            m_internalRegisters[addr - 0x4FFFE0] &= 0xFF00;
            m_internalRegisters[addr - 0x4FFFE0] |= data;
        }
        LOG(if(flags.log && m_cdi.m_callbacks.HasOnLogMemoryAccess()) \
                m_cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::VDSC, "Set", "Byte", m_cdi.m_cpu.currentPC, addr, data});)
        return;
    }

    LOG(if(flags.log && m_cdi.m_callbacks.HasOnLogMemoryAccess()) \
            m_cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Set", "Byte", m_cdi.m_cpu.currentPC, addr, data});)

    throw SCC68070::Exception(SCC68070::BusError);
}

void MCD212::SetWord(const uint32_t addr, const uint16_t data, const BusFlags flags)
{
    if(addr < 0x400000)
    {
        m_memory[addr]     = bits<8, 15>(data);
        m_memory[addr + 1] = data;
        LOG(if(flags.log && m_cdi.m_callbacks.HasOnLogMemoryAccess()) \
                m_cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::RAM, "Set", "Word", m_cdi.m_cpu.currentPC, addr, data});)
        return;
    }

    if(addr >= 0x4FFFE0 && addr < 0x500000)
    {
        // TODO: when writing to the registers, make the change to the renderer too.
        m_internalRegisters[addr - 0x4FFFE0] = data;
        LOG(if(flags.log && m_cdi.m_callbacks.HasOnLogMemoryAccess()) \
                m_cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::VDSC, "Set", "Word", m_cdi.m_cpu.currentPC, addr, data});)
        return;
    }

    LOG(if(flags.log && m_cdi.m_callbacks.HasOnLogMemoryAccess()) \
            m_cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Set", "Word", m_cdi.m_cpu.currentPC, addr, data});)

    throw SCC68070::Exception(SCC68070::BusError);
}
