#ifndef CDI_CORES_SCC68070_SCC68070_HPP
#define CDI_CORES_SCC68070_SCC68070_HPP

class CDI;
#include "../../common/types.hpp"

#include "m68000.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class SCC68070
{
public:
    static constexpr size_t PAL_FREQUENCY = 15'000'000;
    static constexpr size_t NTSC_FREQUENCY = 15'104'900;

    enum class Register
    {
        D0 = 0,
        D1,
        D2,
        D3,
        D4,
        D5,
        D6,
        D7,

        A0 = 8,
        A1,
        A2,
        A3,
        A4,
        A5,
        A6,
        A7,

        PC,
        SR,
        SSP,
        USP,
    };

    enum ExceptionVector : uint8_t
    {
        ResetSSPPC = 0,
        BusError = 2,
        AddressError,
        IllegalInstruction,
        ZeroDivide,
        CHKInstruction,
        TRAPVInstruction,
        PrivilegeViolation,
        Trace,
        Line0101Emulator,
        Line1111Emulator,
        FormatError = 14,
        UninitializedInterrupt,
        SpuriousInterrupt = 24,
        Level1ExternalInterruptAutovector,
        Level2ExternalInterruptAutovector,
        Level3ExternalInterruptAutovector,
        Level4ExternalInterruptAutovector,
        Level5ExternalInterruptAutovector,
        Level6ExternalInterruptAutovector,
        Level7ExternalInterruptAutovector,
        Trap0Instruction,
        Trap1Instruction,
        Trap2Instruction,
        Trap3Instruction,
        Trap4Instruction,
        Trap5Instruction,
        Trap6Instruction,
        Trap7Instruction,
        Trap8Instruction,
        Trap9Instruction,
        Trap10Instruction,
        Trap11Instruction,
        Trap12Instruction,
        Trap13Instruction,
        Trap14Instruction,
        Trap15Instruction,
        Level1OnChipInterruptAutovector = 57,
        Level2OnChipInterruptAutovector,
        Level3OnChipInterruptAutovector,
        Level4OnChipInterruptAutovector,
        Level5OnChipInterruptAutovector,
        Level6OnChipInterruptAutovector,
        Level7OnChipInterruptAutovector,
        UserInterrupt,
    };

    struct Exception
    {
        uint8_t vector;
        uint8_t priority; /**< Group and priority merged. */
        uint16_t data;

        Exception() = delete;
        explicit Exception(const uint8_t vec, const uint16_t d = 0) : vector(vec), priority(GetPriority(vec)), data(d) {}

        bool operator<(const Exception& other) const
        {
            return this->priority < other.priority;
        }

    private:
        static constexpr uint8_t GetPriority(const uint8_t vec)
        {
            if(vec == ResetSSPPC) return 0;
            if(vec == AddressError) return 1;
            if(vec == BusError) return 2;
            if(vec == Trace) return 3;
            if((vec >= SpuriousInterrupt && vec <= Level7ExternalInterruptAutovector) ||
               (vec >= Level1OnChipInterruptAutovector && vec <= Level7OnChipInterruptAutovector) ||
               vec >= UserInterrupt) return 4;
            if(vec == IllegalInstruction) return 5;
            if(vec == PrivilegeViolation) return 6;
            return 7; // Instruction exceptions.
        }
    };

    uint32_t currentPC;
    uint64_t totalCycleCount;

    std::vector<uint32_t> breakpoints;

    SCC68070() = delete;
    SCC68070(SCC68070&) = delete;
    SCC68070(SCC68070&&) = delete;
    SCC68070(CDI& idc, const uint32_t clockFrequency);
    ~SCC68070();

    bool IsRunning() const;
    void SetEmulationSpeed(const double speed);

    void Run(const bool loop = true);
    void Stop(const bool wait = true);
    void Reset();

    void INT1();
    void INT2();
    void IN2();
    void SendUARTIn(const uint8_t byte);

    void SetRegister(Register reg, const uint32_t value);
    std::map<Register, uint32_t> GetCPURegisters() const;
    std::vector<InternalRegister> GetInternalRegisters() const;

private:
    enum Peripheral : uint32_t
    {
        Base = 0x80001001,
        Last = 0x80008080,
        Size = Last - Base,

        LIR  = 0x80001001 - Base, // Latched Interrupt priority level Register

        IDR  = 0x80002001 - Base, // I2C Data Register
        IAR  = 0x80002003 - Base, // I2C Address Register
        ISR  = 0x80002005 - Base, // I2C Status Register
        ICR  = 0x80002007 - Base, // I2C Control Register
        ICCR = 0x80002009 - Base, // I2C Clock Control Register

        UMR  = 0x80002011 - Base, // UART Mode Register
        USR  = 0x80002013 - Base, // UART Status Register
        UCSR = 0x80002015 - Base, // UART Clock Select Register
        UCR  = 0x80002017 - Base, // UART Command Register
        UTHR = 0x80002019 - Base, // UART Transmit Holding Register
        URHR = 0x8000201B - Base, // UART Receive Holding Register

        TSR = 0x80002020 - Base, // Timer Status Register
        TCR = 0x80002021 - Base, // Timer Control Register
        RRH = 0x80002022 - Base, // Reload Register High
        RRL = 0x80002023 - Base, // Reload Register Low
        T0H = 0x80002024 - Base, // Timer 0 High
        T0L = 0x80002025 - Base, // Timer 0 Low
        T1H = 0x80002026 - Base, // Timer 1 High
        T1L = 0x80002027 - Base, // Timer 1 Low
        T2H = 0x80002028 - Base, // Timer 2 High
        T2L = 0x80002029 - Base, // Timer 2 Low

        PICR1 = 0x80002045 - Base, // Peripheral Interrupt Control Register 1
        PICR2 = 0x80002047 - Base, // Peripheral Interrupt Control Register 2

        // DMA channel 1
        CSR1   = 0x80004000 - Base, // Channel Status Register 1
        CER1   = 0x80004001 - Base, // Channel Error Register 1
        DCR1   = 0x80004004 - Base, // Device Control Register 1
        OCR1   = 0x80004005 - Base, // Operation Control Register 1
        SCR1   = 0x80004006 - Base, // Sequence Control Register 1
        CCR1   = 0x80004007 - Base, // Channel Control Register 1
        MTCH1  = 0x8000400A - Base, // Memory Transfer Counter High 1
        MTCL1  = 0x8000400B - Base, // Memory Transfer Counter Low 1
        MACH1  = 0x8000400C - Base, // Memory Address Counter High 1
        MACMH1 = 0x8000400D - Base, // Memory Address Counter Middle High 1
        MACML1 = 0x8000400E - Base, // Memory Address Counter Middle Low 1
        MACL1  = 0x8000400F - Base, // Memory Address Counter Low 1
        CPR1   = 0x8000402D - Base, // Channel Priority Register 1

        // DMA channel 2
        CSR2   = 0x80004040 - Base, // Channel Status Register 2
        CER2   = 0x80004041 - Base, // Channel Error Register 2
        DCR2   = 0x80004044 - Base, // Device Control Register 2
        OCR2   = 0x80004045 - Base, // Operation Control Register 2
        SCR2   = 0x80004046 - Base, // Sequence Control Register 2
        CCR2   = 0x80004047 - Base, // Channel Control Register 2
        MTCH2  = 0x8000404A - Base, // Memory Transfer Counter High 2
        MTCL2  = 0x8000404B - Base, // Memory Transfer Counter Low 2
        MACH2  = 0x8000404C - Base, // Memory Address Counter High 2
        MACMH2 = 0x8000404D - Base, // Memory Address Counter Middle High 2
        MACML2 = 0x8000404E - Base, // Memory Address Counter Middle Low 2
        MACL2  = 0x8000404F - Base, // Memory Address Counter Low 2
        DACH2  = 0x80004054 - Base, // Device Address Counter High 2
        DACMH2 = 0x80004055 - Base, // Device Address Counter Middle High 2
        DACML2 = 0x80004056 - Base, // Device Address Counter Middle Low 2
        DACL2  = 0x80004057 - Base, // Device Address Counter Low 2
        CPR2   = 0x8000406D - Base, // Channel Priority Register 2

        MSR  = 0x80008000 - Base, // MMU Status Register
        MCR  = 0x80008001 - Base, // MMU Control Register
        ATTR = 0x80008040 - Base,
        SEG_LENGTH = 0x80008042 - Base,
        SEG_NUMBER = 0x80008045 - Base,
        BASE_ADDRESS = 0x80008046 - Base,
    };

    CDI& cdi;
    std::thread executionThread;

    M68000* m68000;
    M68000Callbacks m68000Callbacks;

    std::mutex uartInMutex;
    std::deque<uint8_t> uartIn;

    std::atomic_bool loop;
    std::atomic_bool isRunning;

    const double cycleDelay; // Time between two clock cycles in nanoseconds
    double speedDelay; // used for emulation speed.
    const double timerDelay;
    double timerCounter; // Counts the nanosconds when incrementing the timer.

    std::array<uint8_t, Peripheral::Size> internal;

    // Direct Memory Access
    friend GetSetResult get_byte(uint32_t);
    friend GetSetResult get_word(uint32_t);
    friend GetSetResult get_long(uint32_t);
    friend GetSetResult set_byte(uint32_t, uint8_t);
    friend GetSetResult set_word(uint32_t, uint16_t);
    friend GetSetResult set_long(uint32_t, uint32_t);
    friend void reset_instruction();
    friend void disassembler(uint32_t, const char*);

    uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Log | Trigger);
    uint16_t GetWord(const uint32_t addr, const uint8_t flags = Log | Trigger);
    uint32_t GetLong(const uint32_t addr, const uint8_t flags = Log | Trigger);

    void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Log | Trigger);
    void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Log | Trigger);
    void SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags = Log | Trigger);

    uint16_t GetNextWord(const uint8_t flags = Log | Trigger);
    uint16_t PeekNextWord();

    // Peripherals
    uint8_t GetPeripheral(uint32_t addr);
    void SetPeripheral(uint32_t addr, const uint8_t data);
    void IncrementTimer(const double ns);

    // Instruction Set
    void Interpreter();
    void ResetOperation();
};

inline const char* CPURegisterToString(const SCC68070::Register reg)
{
    switch(reg)
    {
    case SCC68070::Register::D0: return "D0";
    case SCC68070::Register::D1: return "D1";
    case SCC68070::Register::D2: return "D2";
    case SCC68070::Register::D3: return "D3";
    case SCC68070::Register::D4: return "D4";
    case SCC68070::Register::D5: return "D5";
    case SCC68070::Register::D6: return "D6";
    case SCC68070::Register::D7: return "D7";

    case SCC68070::Register::A0: return "A0";
    case SCC68070::Register::A1: return "A1";
    case SCC68070::Register::A2: return "A2";
    case SCC68070::Register::A3: return "A3";
    case SCC68070::Register::A4: return "A4";
    case SCC68070::Register::A5: return "A5";
    case SCC68070::Register::A6: return "A6";
    case SCC68070::Register::A7: return "A7";

    case SCC68070::Register::PC: return "PC";
    case SCC68070::Register::SR: return "SR";
    case SCC68070::Register::USP: return "USP";
    case SCC68070::Register::SSP: return "SSP";
    default: return "Unknown";
    }
}

std::string exceptionVectorToString(uint8_t vector);

#define   SET_TX_READY() internal[USR] |= 0x04;
#define   SET_RX_READY() internal[USR] |= 0x01;
#define UNSET_TX_READY() internal[USR] &= ~0x04;
#define UNSET_RX_READY() internal[USR] &= ~0x01;

#define RESET_INTERNAL() { internal = {0}; SET_TX_READY() }
#define CLEAR_PRIORITY_QUEUE(queue) while(queue.size()) queue.pop();

#endif // CDI_CORES_SCC68070_SCC68070_HPP
