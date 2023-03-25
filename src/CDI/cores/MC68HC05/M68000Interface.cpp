#include "M68000Interface.hpp"

#include <cstdio>

#define NIBBLE_SWAP(val) (uint8_t)((val >> 4 & 0x0F) | (val << 4))
#define INT_STATUS_MASK_SWAP(val) ((val >> 1 & 0x55) | (val << 1 & 0xAA))

M68000Interface::M68000Interface()
    : modeMCU(0)
    , channelReadMCU{{}}
    , channelWriteMCU{{}}
    , channelStatusMCU{0x11, 0x11, 0x11, 0x11}
    , interruptStatus(0)
    , interruptMask(0)
    , modeHost(0)
{
}

void M68000Interface::Reset()
{
    ClearChannels();

    interruptMask = 0;
    modeMCU = 0;
    modeHost = 0;
}

uint8_t M68000Interface::PopByteMCU(size_t channel)
{
    if(!ChannelEnabledMCU(channel))
        return 0;

    if(channelReadMCU[channel].size() == 0)
    {
        printf("[%s] Pop MCU with size 0.", __FUNCTION__);
        return 0;
    }

    const uint8_t data = channelReadMCU[channel].front();
    channelReadMCU[channel].pop_front();

    channelStatusMCU[channel] &= ~RFULL;
    if(channelReadMCU[channel].size() == 0)
    {
        channelStatusMCU[channel] |= REMTY;
        channelStatusMCU[channel] &= ~RRDY;

        constexpr uint8_t FLAGS[4] = {RA, RB, RC, RD};
        interruptStatus &= ~FLAGS[channel];
    }

    return data;
}

/** \brief Called by the MCU to push a byte in its transmit channel.
 * \return true if a host interrupt has to be triggered, false otherwise.
 */
bool M68000Interface::PushByteMCU(size_t channel, uint8_t data)
{
    if(!ChannelEnabledMCU(channel))
        return false;

    if(channelWriteMCU[channel].size() >= 4)
    {
        channelStatusMCU[channel] |= TOR; // Overrun.
        printf("[%s] Push MCU with size 4.", __FUNCTION__);
        return false;
    }

    channelWriteMCU[channel].push_back(data);

    bool interrupt = false;
    constexpr uint8_t FLAGS[4] = {TA, TB, TC, TD};
    constexpr uint8_t MASKS[4] = {TMA, TMB, TMC, TMD};

    channelStatusMCU[channel] |= TRDY;
    channelStatusMCU[channel] &= ~TEMTY;
    if(channelWriteMCU[channel].size() >= 4)
    {
        channelStatusMCU[channel] |= TFULL;
        if(!(modeHost & IMOD))
        {
            interruptStatus |= FLAGS[channel];
            if(interruptMask & MASKS[channel])
                interrupt = true;
        }
    }

    if(modeHost & IMOD)
    {
        interruptStatus |= FLAGS[channel];
        if(interruptMask & MASKS[channel])
            interrupt = true;
    }

    return interrupt;
}

uint8_t M68000Interface::GetStatusMCU(size_t channel)
{
    const uint8_t status = channelStatusMCU[channel];
    channelStatusMCU[channel] &= ~TOR;
    return status;
}

uint8_t M68000Interface::GetInterruptStatusMCU()
{
    return interruptStatus;
}

uint8_t M68000Interface::GetInterruptMaskMCU()
{
    return interruptMask;
}

void M68000Interface::SetInterruptMaskMCU(uint8_t value)
{
    interruptMask &= 0x55;
    interruptMask |= value & 0xAA;
}

uint8_t M68000Interface::PopByteHost(size_t channel)
{
    if(!ChannelEnabledHost(channel))
        return 0;

    if(channelWriteMCU[channel].size() == 0)
    {
        printf("[%s] Pop host with size 0.", __FUNCTION__);
        return 0;
    }

    const uint8_t data = channelWriteMCU[channel].front();
    channelWriteMCU[channel].pop_front();

    channelStatusMCU[channel] &= ~TFULL;
    if(channelWriteMCU[channel].size() == 0)
    {
        channelStatusMCU[channel] |= TEMTY;
        channelStatusMCU[channel] &= ~TRDY;

        constexpr uint8_t FLAGS[4] = {TA, TB, TC, TD};
        interruptStatus &= ~FLAGS[channel];
    }

    return data;
}

/** \brief Called by the host to push a byte in its transmit channel.
 * \return true if a MCU interrupt has to be triggered, false otherwise.
 */
bool M68000Interface::PushByteHost(size_t channel, uint8_t data)
{
    if(!ChannelEnabledHost(channel))
        return false;

    if(channelReadMCU[channel].size() >= 4)
    {
        channelStatusMCU[channel] |= ROR; // Overrun
        printf("[%s] Push host with size 4.", __FUNCTION__);
        return false;
    }

    channelReadMCU[channel].push_back(data);

    bool interrupt = false;
    constexpr uint8_t FLAGS[4] = {RA, RB, RC, RD};
    constexpr uint8_t MASKS[4] = {RMA, RMB, RMC, RMD};

    channelStatusMCU[channel] |= RRDY;
    channelStatusMCU[channel] &= ~REMTY;
    if(channelReadMCU[channel].size() >= 4)
    {
        channelStatusMCU[channel] |= RFULL;
        if(!(modeMCU & IMOD))
        {
            interruptStatus |= FLAGS[channel];
            if(interruptMask & MASKS[channel])
                interrupt = true;
        }
    }

    if(modeMCU & IMOD)
    {
        interruptStatus |= FLAGS[channel];
        if(interruptMask & MASKS[channel])
            interrupt = true;
    }

    return interrupt;
}

uint8_t M68000Interface::GetStatusHost(size_t channel)
{
    const uint8_t status = channelStatusMCU[channel];
    channelStatusMCU[channel] &= ~ROR;
    return NIBBLE_SWAP(status);
}

uint8_t M68000Interface::GetInterruptStatusHost()
{
    return INT_STATUS_MASK_SWAP(interruptStatus);
}

uint8_t M68000Interface::GetInterruptMaskHost()
{
    return INT_STATUS_MASK_SWAP(interruptMask);
}

void M68000Interface::SetInterruptMaskHost(uint8_t value)
{
    interruptMask &= 0xAA;
    interruptMask |= INT_STATUS_MASK_SWAP(value) & 0x55;
}

uint8_t M68000Interface::GetModeHost()
{
    return modeHost;
}

void M68000Interface::SetModeHost(uint8_t value)
{
    if(value & FCLR) // Clear all channels.
        ClearChannels();

    modeHost = value & 0x8F; // Do not set the FCLR bit.
}

void M68000Interface::ClearChannels()
{
    for(std::deque<uint8_t>& channel : channelReadMCU)
        channel.clear();
    for(std::deque<uint8_t>& channel : channelWriteMCU)
        channel.clear();
    channelStatusMCU.fill(0x11);
    interruptStatus = 0;
}

bool M68000Interface::ChannelEnabledMCU(size_t channel)
{
    constexpr uint8_t FLAGS[4] = {AEN, BEN, CEN, DEN};
    return modeMCU & FLAGS[channel];
}

bool M68000Interface::ChannelEnabledHost(size_t channel)
{
    constexpr uint8_t FLAGS[4] = {AEN, BEN, CEN, DEN};
    return modeHost & FLAGS[channel];
}
