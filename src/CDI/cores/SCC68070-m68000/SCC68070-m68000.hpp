#ifndef CDI_CORES_SCC68070_M68000_SCC68070_M68000_HPP
#define CDI_CORES_SCC68070_M68000_SCC68070_M68000_HPP

class CDI;
#include "../../common/types.hpp"
#include "../../common/utils.hpp"

extern "C"
{
#include <m68000/m68000-ffi.h>
}

#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

class M68000Deleter
{
public:
    void operator()(m68000_scc68070_t* ptr) const
    {
        m68000_scc68070_delete(ptr);
    }
};

m68000_memory_result_t getByte(uint32_t addr, void* user_data);
m68000_memory_result_t getWord(uint32_t addr, void* user_data);
m68000_memory_result_t getLong(uint32_t addr, void* user_data);
m68000_memory_result_t setByte(uint32_t addr, uint8_t data, void* user_data);
m68000_memory_result_t setWord(uint32_t addr, uint16_t data, void* user_data);
m68000_memory_result_t setLong(uint32_t addr, uint32_t data, void* user_data);
void resetInstruction(void* user_data);

class SCC68070
{
public:
    static constexpr size_t PAL_FREQUENCY = 15'000'000;
    static constexpr size_t NTSC_FREQUENCY = 15'104'900;

    static constexpr m68000_vector_t BusError = AccessError;
    static constexpr m68000_vector_t Trap0Instruction = m68000_vector_t::Trap0Instruction;
    static constexpr m68000_vector_t Trap15Instruction = m68000_vector_t::Trap15Instruction;

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

        USP,
        SSP,
        PC,
        SR,
    };

    struct Exception
    {
        m68000_vector_t vector;
        uint8_t priority; /**< Group and priority. */
        uint16_t data;

        Exception() = delete;
        Exception(const m68000_vector_t vec, const uint16_t d = 0) : vector(vec), priority(GetPriority(vec)), data(d) {}

        bool operator<(const Exception& other) const
        {
            return this->priority < other.priority;
        }

    private:
        static constexpr uint8_t GetPriority(const m68000_vector_t vec)
        {
            if(vec == ResetSspPc) return 0;
            if(vec == AddressError) return 1;
            if(vec == AccessError) return 2;
            if(vec == Trace) return 3;
            if((vec >= SpuriousInterrupt && vec <= Level7Interrupt) ||
               (vec >= Level1OnChipInterrupt && vec <= Level7OnChipInterrupt) ||
               vec >= UserInterrupt) return 4;
            if(vec == IllegalInstruction) return 5;
            if(vec == PrivilegeViolation) return 6;
            return 7; // Instruction exceptions.
        }
    };

    uint32_t currentPC;
    uint64_t totalCycleCount;

    std::vector<uint32_t> breakpoints;

    SCC68070(CDI& idc, uint32_t clockFrequency);
    ~SCC68070();

    bool IsRunning() const;
    void SetEmulationSpeed(double speed);

    void Run(bool loop = true);
    void Stop(bool wait = true);
    void Reset();

    void INT1();
    void INT2();
    void IN2();
    void SendUARTIn(uint8_t byte);

    /** \brief Returns the byte at the given peripheral address.
     * \param addr The address in peripheral memory (so between 0 and Peripheral::Last).
     */
    uint8_t PeekPeripheral(uint32_t addr) const noexcept;

    // void SetRegister(Register reg, uint32_t value);
    std::map<Register, uint32_t> GetCPURegisters() const;
    std::vector<InternalRegister> GetInternalRegisters() const;

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

private:
    friend m68000_memory_result_t getByte(uint32_t addr, void* user_data);
    friend m68000_memory_result_t getWord(uint32_t addr, void* user_data);
    friend m68000_memory_result_t getLong(uint32_t addr, void* user_data);
    friend m68000_memory_result_t setByte(uint32_t addr, uint8_t data, void* user_data);
    friend m68000_memory_result_t setWord(uint32_t addr, uint16_t data, void* user_data);
    friend m68000_memory_result_t setLong(uint32_t addr, uint32_t data, void* user_data);
    friend void resetInstruction(void* user_data);

    CDI& m_cdi;
    std::thread m_executionThread;

    std::mutex m_uartInMutex;
    std::deque<uint8_t> m_uartIn;

    std::atomic_bool m_loop;
    std::atomic_bool m_isRunning;

    std::unique_ptr<m68000_scc68070_t, M68000Deleter> m_m68000;
    m68000_callbacks_t m_memory;
    const m68000_registers_t* m_regs;

    // void DumpCPURegisters();

    const double m_cycleDelay; // Time between two clock cycles in nanoseconds
    double m_speedDelay; // used for emulation speed.
    const double m_timerDelay;
    double m_timerCounter; // Counts the nanosconds when incrementing the timer.

    // Internal
    void ResetInternal();
    std::array<uint8_t, Peripheral::Size> m_peripherals;

    // Direct Memory Access
    uint8_t  GetByte(uint32_t addr, BusFlags flags = BUS_NORMAL);
    uint16_t GetWord(uint32_t addr, BusFlags flags = BUS_NORMAL);
    uint32_t GetLong(uint32_t addr, BusFlags flags = BUS_NORMAL);

    void SetByte(uint32_t addr, uint8_t  data, BusFlags flags = BUS_NORMAL);
    void SetWord(uint32_t addr, uint16_t data, BusFlags flags = BUS_NORMAL);
    void SetLong(uint32_t addr, uint32_t data, BusFlags flags = BUS_NORMAL);

    // Peripherals
    uint8_t GetPeripheral(uint32_t addr, BusFlags flags);
    void SetPeripheral(uint32_t addr, uint8_t data, BusFlags flags);
    void IncrementTimer(double ns);

    // Instruction Set
    void Interpreter();
    void ResetOperation();
};

constexpr const char* CPURegisterToString(const SCC68070::Register reg)
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

#define   SET_TX_READY() m_peripherals[USR] |= 0x04;
#define   SET_RX_READY() m_peripherals[USR] |= 0x01;
#define UNSET_TX_READY() m_peripherals[USR] &= ~0x04;
#define UNSET_RX_READY() m_peripherals[USR] &= ~0x01;

#endif // CDI_CORES_SCC68070_M68000_SCC68070_M68000_HPP
