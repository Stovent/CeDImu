#include "IKAT.hpp"
#include "../../CDI.hpp"

LOG(static std::string getPortName(uint8_t index)
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
})

namespace HLE
{

#define UNSET_REMTY(reg) registers[reg] &= 0x01;
#define SET_RTEMTY(reg) registers[reg] |= 0x11;
#define CHANNEL(addr) (addr % 4)

#define SET_INT(reg) registers[ISR] |= INT_MASK[reg]; \
                     if(registers[IMR] & INT_MASK[reg]) \
                         cdi.board->cpu.IN2();

#define DELAY_RSP(channel, rsp) { delayedRsp[channel] = &rsp; \
                                delayedRspFrame[channel] = cdi.board->GetTotalFrameCount() + 2; }

IKAT::IKAT(CDI& idc, bool PAL, uint32_t busbase, PointingDevice::Type deviceType)
    : ISlave(idc, busbase, deviceType)
    , registers{0}
    , delayedRsp{nullptr}
    , delayedRspFrame{0}
{
    responseCF6[2] = PAL + 1;

    for(int reg = CHA_SR; reg <= CHD_SR; reg++)
        SET_RTEMTY(reg)
}

void IKAT::UpdatePointerState()
{
    channelOut[CHB].clear();
    channelOut[CHB].insert(channelOut[CHB].begin(), pointingDevice.pointerMessage.begin(), pointingDevice.pointerMessage.end());

    UNSET_REMTY(CHB_SR)
    SET_INT(CHB)
}

void IKAT::IncrementTime(const size_t ns)
{
    pointingDevice.IncrementTime(ns);

    for(int channel = CHA; channel <= CHD; channel++)
    {
        if(delayedRsp[channel] != nullptr && delayedRspFrame[channel] == cdi.board->GetTotalFrameCount())
        {
            channelOut[channel].clear();
            channelOut[channel].insert(channelOut[channel].begin(), delayedRsp[channel]->begin(), delayedRsp[channel]->end());

            UNSET_REMTY(CHA_SR + channel)
            SET_INT(channel)

            delayedRsp[channel] = nullptr;
            delayedRspFrame[channel] = 0;
        }
    }
}

uint8_t IKAT::GetByte(const uint8_t addr)
{
    const uint8_t channel = CHANNEL(addr);
    if(addr >= CHA_OUT && addr <= CHD_OUT && channelOut[channel].size() > 0)
    {
        registers[addr] = channelOut[channel].front();
        channelOut[channel].pop_front();

        // remove interrupt bit if the response is completely read.
        if(channelOut[channel].size() == 0)
        {
            registers[ISR] &= ~(1 << (1 + 2 * channel));
            SET_RTEMTY(CHA_SR + channel)
        }
        else
            UNSET_REMTY(CHA_SR + channel)
    }

    LOG(if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::Slave, "Get", getPortName(addr), cdi.board->cpu.currentPC, busBase + (addr << 1) + 1, registers[addr]});)

    return registers[addr];
}

void IKAT::SetByte(const uint8_t addr, const uint8_t data)
{
    LOG(if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::Slave, "Set", getPortName(addr), cdi.board->cpu.currentPC, busBase + (addr << 1) + 1, data});)

    if(addr == IMR)
        registers[addr] = data;
    else if(addr >= CHA_IN && addr <= CHD_IN)
    {
        registers[addr] = data;
        channelIn[CHANNEL(addr)].push_back(data);

        switch(addr)
        {
        case CHC_IN:
            ProcessCommandC();
            break;

        case CHD_IN:
            ProcessCommandD();
            break;
        }
    }
}

void IKAT::ProcessCommandC()
{
    switch(channelIn[CHC][0])
    {
    case 0xF0:
        channelIn[CHC].clear();
        channelOut[CHC].clear();
        channelOut[CHC].insert(channelOut[CHC].begin(), responseCF0.begin(), responseCF0.end());

        UNSET_REMTY(CHC_SR)
        SET_INT(CHC)
        break;

    case 0xF4:
        channelIn[CHC].clear();
        channelOut[CHC].clear();
        channelOut[CHC].insert(channelOut[CHC].begin(), responseCF4.begin(), responseCF4.end());

        UNSET_REMTY(CHC_SR)
        SET_INT(CHC)
        break;

    case 0xF6:
        channelIn[CHC].clear();
        channelOut[CHC].clear();
        channelOut[CHC].insert(channelOut[CHC].begin(), responseCF6.begin(), responseCF6.end());

        UNSET_REMTY(CHC_SR)
        SET_INT(CHC)
        break;

    case 0xF7:
        channelIn[CHC].clear();
        channelOut[CHC].clear();
        channelOut[CHC].insert(channelOut[CHC].begin(), responseCF7.begin(), responseCF7.end());

        UNSET_REMTY(CHC_SR)
        SET_INT(CHC)
        break;

    case 0xF8:
        channelIn[CHC].clear();
        channelOut[CHC].clear();
        channelOut[CHC].insert(channelOut[CHC].begin(), responseCF8.begin(), responseCF8.end());

        UNSET_REMTY(CHC_SR)
        SET_INT(CHC)
        break;

    default:
        channelIn[CHC].clear();
    }
}

void IKAT::ProcessCommandD()
{
    switch(channelIn[CHD][0])
    {
    case 0xA1:
        if(channelIn[CHD].size() == 4)
        {
            channelIn[CHD].clear();
            DELAY_RSP(CHD, responseDB0)
        }
        break;

    case 0xB0:
        if(channelIn[CHD].size() == 4)
        {
            channelIn[CHD].clear();
            DELAY_RSP(CHD, responseDB0)
        }
        break;

    case 0xB1:
        channelIn[CHD].clear();
        channelOut[CHD].clear();
        channelOut[CHD].insert(channelOut[CHD].begin(), responseDB1.begin(), responseDB1.end());

        UNSET_REMTY(CHD_SR)
        SET_INT(CHD)
        break;

    case 0xB2:
        if(channelIn[CHD].size() == 4)
        {
            channelIn[CHD].clear();
            DELAY_RSP(CHD, responseDB2)
        }
        break;

    default:
        channelIn[CHD].clear();
    }
}

} // namespace HLE
