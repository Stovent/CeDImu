#include "IKAT.hpp"
#include "../../CDI.hpp"

static std::string getPortName(uint8_t index)
{
    std::string port;
    if(index < 12)
    {
        port = 'A' + index % 4;
        if(index < 4)
            port += " write";
        else if(index < 8)
            port += " read";
        else
            port += " status";
    }
    else if(index == 12)
        port = "ISR";
    else if(index == 13)
        port = "ICR";
    else if(index == 14)
        port = "YCR";

    return port;
}

namespace HLE
{

#define UNSET_RDIDLE(var) var &= 0x01;
#define UNSET_WRIDLE(var) var &= 0x10;
#define SET_RDWRIDLE(var) var = 0x11;

IKAT::IKAT(CDI& idc, const bool PAL, uint32_t busbase) : ISlave(idc, busbase)
{
    responseCF6[2] = PAL + 1;

    for(int i = PASR; i <= PDSR; i++)
        SET_RDWRIDLE(registers[i])
}

void IKAT::UpdatePointerState()
{
    responsesEnd[PB] = std::copy(pointingDevice->pointerMessage.begin(), pointingDevice->pointerMessage.end(), responseB4X.begin());
    responsesIterator[PB] = responseB4X.begin();
    registers[ISR] |= 0x08;
    if(registers[ICR] & 0x08)
        cdi.board->cpu.IN2();
}

void IKAT::IncrementTime(const size_t ns)
{
    pointingDevice->IncrementTime(ns);
}

uint8_t IKAT::GetByte(const uint8_t addr)
{
    if(addr >= PASR && addr <= PDSR)
    {
        for(int i = PA; i <= PD; i++)
            if(responsesIterator[i] != responsesEnd[i] && i != PB && i != PD)
                UNSET_RDIDLE(registers[PASR + i])
            else
                SET_RDWRIDLE(registers[PASR + i])

        LOG(if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({Slave, "Get", getPortName(addr), cdi.board->cpu.currentPC, busBase + (addr << 1) + 1, registers[addr]});)
        return registers[addr];
    }

    const uint8_t reg = addr % 4;
    if(addr >= PARD && addr <= PDRD && responsesIterator[reg] != responsesEnd[reg])
    {
        registers[PARD + reg] = *responsesIterator[reg]++;
    }

    LOG(if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({Slave, "Get", getPortName(addr), cdi.board->cpu.currentPC, busBase + (addr << 1) + 1, registers[addr]});)
    return registers[addr];
}

void IKAT::SetByte(const uint8_t addr, const uint8_t data)
{
    registers[addr] = data;
    LOG(if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({Slave, "Set", getPortName(addr), cdi.board->cpu.currentPC, busBase + (addr << 1) + 1, data});)

    switch(addr)
    {
    case PCWR:
        ProcessCommandC(data);
        break;

    case PDWR:
        ProcessCommandD(data);
        break;
    }
}

void IKAT::ProcessCommandC(uint8_t data)
{
    switch(data)
    {
    case 0xF4:
        responsesIterator[PC] = responseCF4.begin();
        responsesEnd[PC] = responseCF4.end();
        UNSET_WRIDLE(registers[PCSR])
        break;

    case 0xF6:
        responsesIterator[PC] = responseCF6.begin();
        responsesEnd[PC] = responseCF6.end();
        UNSET_WRIDLE(registers[PCSR])
        break;
    }
}

void IKAT::ProcessCommandD(uint8_t data)
{
    switch(data)
    {
    case 0xB0:
        responsesIterator[PD] = responseDB0.begin();
        responsesEnd[PD] = responseDB0.end();
        UNSET_WRIDLE(registers[PDSR])
        break;

    case 0xB1:
        responsesIterator[PD] = responseDB1.begin();
        responsesEnd[PD] = responseDB1.end();
        UNSET_WRIDLE(registers[PDSR])
        break;

    case 0xB2:
        responsesIterator[PD] = responseDB2.begin();
        responsesEnd[PD] = responseDB2.end();
        UNSET_WRIDLE(registers[PDSR])
        break;
    }
}

} // namespace HLE
