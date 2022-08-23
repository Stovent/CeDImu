#ifndef CDI_CORES_MC68HC05_M68000INTERFACE_HPP
#define CDI_CORES_MC68HC05_M68000INTERFACE_HPP

#include <array>
#include <cstdint>
#include <deque>

class M68000Interface
{
public:
    enum Channels
    {
        ChannelA,
        ChannelB,
        ChannelC,
        ChannelD,
    };

    uint8_t modeMCU;

    M68000Interface();

    void Reset();

    [[nodiscard]] uint8_t PopByteMCU(size_t channel);
    [[nodiscard]] bool PushByteMCU(size_t channel, uint8_t data);
    [[nodiscard]] uint8_t GetStatusMCU(size_t channel);
    [[nodiscard]] uint8_t GetInterruptStatusMCU();
    [[nodiscard]] uint8_t GetInterruptMaskMCU();
    void SetInterruptMaskMCU(uint8_t value);

    [[nodiscard]] uint8_t PopByteHost(size_t channel);
    [[nodiscard]] bool PushByteHost(size_t channel, uint8_t data);
    [[nodiscard]] uint8_t GetStatusHost(size_t channel);
    [[nodiscard]] uint8_t GetInterruptStatusHost();
    [[nodiscard]] uint8_t GetInterruptMaskHost();
    void SetInterruptMaskHost(uint8_t value);
    [[nodiscard]] uint8_t GetModeHost();
    void SetModeHost(uint8_t value);

private:
    std::array<std::deque<uint8_t>, 4> channelReadMCU;
    std::array<std::deque<uint8_t>, 4> channelWriteMCU;
    std::array<uint8_t, 4> channelStatusMCU;
    uint8_t interruptStatus;
    uint8_t interruptMask;
    uint8_t modeHost;

    void ClearChannels();
    bool ChannelEnabledMCU(size_t channel);
    bool ChannelEnabledHost(size_t channel);

    enum RegistersFlags
    {
        // Status
        TEMTY = 0x01,
        TFULL = 0x02,
        TRDY  = 0x04,
        TOR   = 0x08,
        REMTY = 0x10,
        RFULL = 0x20,
        RRDY  = 0x40,
        ROR   = 0x80,

        // Interrupt Status
        TD = 0x01,
        RD = 0x02,
        TC = 0x04,
        RC = 0x08,
        TB = 0x10,
        RB = 0x20,
        TA = 0x40,
        RA = 0x80,

        // Interrupt Mask
        TMD = 0x01,
        RMD = 0x02,
        TMC = 0x04,
        RMC = 0x08,
        TMB = 0x10,
        RMB = 0x20,
        TMA = 0x40,
        RMA = 0x80,

        // Mode
        DEN  = 0x01,
        CEN  = 0x02,
        BEN  = 0x04,
        AEN  = 0x08,
        FCLR = 0x40,
        IMOD = 0x80,
    };
};

#endif // CDI_CORES_MC68HC05_M68000INTERFACE_HPP
