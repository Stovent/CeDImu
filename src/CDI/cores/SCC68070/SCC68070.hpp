#ifndef CDI_CORES_SCC68070_SCC68070_HPP
#define CDI_CORES_SCC68070_SCC68070_HPP

class CDI;
#include "../../common/types.hpp"
#include "../../common/utils.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <utility>
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

        USP,
        SSP,
        PC,
        SR,
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
        Line1010Emulator,
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
        ExceptionVector vector; /**< The exception vector. */
        uint8_t priority; /**< Group and priority. */
        uint16_t data; /**< Additional data (used to store the syscall in Trap 0 instructions). */

        Exception() = delete;
        Exception(const ExceptionVector vec, const uint16_t d = 0) : vector(vec), priority(GetPriority(vec)), data(d) {}

        bool operator<(const Exception& other) const
        {
            return this->priority < other.priority;
        }

    private:
        static constexpr uint8_t GetPriority(const ExceptionVector vec)
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

    // TODO: I don't like this but this is the quickest interface I can develop now.
    struct Breakpoint
    {
        uint32_t address; /**< The adress of the instruction that caused the breakpoint. */
    };

    uint32_t currentPC;
    uint64_t totalCycleCount;
    const double cycleDelay; // Time between two clock cycles in nanoseconds

    std::vector<uint32_t> breakpoints;
    bool m_breakpointed; /**< set to true when reaching a breakpointed instruction. */

    SCC68070(CDI& idc, uint32_t clockFrequency);
    ~SCC68070() noexcept;

    SCC68070(const SCC68070&) = delete;
    SCC68070(SCC68070&&) = delete;

    size_t SingleStep(size_t stopCycles);
    std::pair<size_t, std::optional<Exception>> SingleStepException(size_t stopCycles);
    void Reset();
    void PushException(const Exception& ex);
    uint16_t GetNextWord(BusFlags flags = BUS_NORMAL);
    uint16_t PeekNextWord() const noexcept;

    void INT1();
    void INT2();
    void IN2();
    void SendUARTIn(uint8_t byte);

    /** \brief Returns the byte at the given peripheral address.
     * \param addr The address in peripheral memory (so between 0 and Peripheral::Last).
     */
    uint8_t PeekPeripheral(uint32_t addr) const noexcept;

    void SetRegister(Register reg, uint32_t value);
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
    CDI& m_cdi;

    std::mutex m_uartInMutex;
    std::deque<uint8_t> m_uartIn;

    bool m_stop;

    void DumpCPURegisters();

    const double m_timerDelay;
    double m_timerCounter; // Counts the nanosconds when incrementing the timer.

    // Internal
    void ResetInternal();
    std::array<uint8_t, Peripheral::Size> m_peripherals;
    uint16_t currentOpcode;
    uint32_t lastAddress;

    // Registers
    uint32_t D[8];
    uint32_t A_[8]; // A_[7] is a dummy register, use A() for safety.
    uint32_t PC;
    uint16_t SR;
    uint32_t USP;
    uint32_t SSP;

    constexpr uint32_t& A(const uint8_t reg) { return reg == 7 ? (GetS() ? SSP : USP) : A_[reg]; }
    constexpr uint32_t A(const uint8_t reg) const { return reg == 7 ? (GetS() ? SSP : USP) : A_[reg]; }

    // Conditional Codes
    constexpr bool GetS() const { return bit<13>(SR); }
    void SetS(bool S = 1);
    constexpr bool GetX() const { return bit<4>(SR); }
    void SetX(bool X = 1);
    constexpr bool GetN() const { return bit<3>(SR); }
    void SetN(bool N = 1);
    constexpr bool GetZ() const { return bit<2>(SR); }
    void SetZ(bool Z = 1);
    constexpr bool GetV() const { return bit<1>(SR); }
    void SetV(bool V = 1);
    constexpr bool GetC() const { return bit<0>(SR); }
    void SetC(bool C = 1);
    void SetXC(bool XC = 1); // Set both X and C at the same time
    void SetVC(bool VC = 1); // Set both V and C at the same time
    constexpr uint8_t GetIPM() const { return bits<8, 10>(SR); }; // Interrupt Priority Mask

    // Exceptions
    std::priority_queue<Exception> m_exceptions;
    void ClearExceptions();

    uint16_t ProcessException(ExceptionVector vector);
    static std::string exceptionVectorToString(ExceptionVector vector);

    // Addressing Modes
    uint32_t GetEffectiveAddress(uint8_t mode, uint8_t reg, uint8_t sizeInBytes, uint16_t& calcTime);
    int32_t GetIndexRegister(uint16_t bew) const;

    uint32_t AddressRegisterIndirectWithPostincrement(uint8_t reg, uint8_t sizeInByte);
    uint32_t AddressRegisterIndirectWithPredecrement(uint8_t reg, uint8_t sizeInByte);
    uint32_t AddressRegisterIndirectWithDisplacement(uint8_t reg);
    uint32_t AddressRegisterIndirectWithIndex8(uint8_t reg);

    uint32_t ProgramCounterIndirectWithDisplacement();
    uint32_t ProgramCounterIndirectWithIndex8();

    uint32_t AbsoluteShortAddressing();
    uint32_t AbsoluteLongAddressing();

    std::string DisassembleAddressingMode(uint32_t extWordAddress, uint8_t eamode, uint8_t eareg, uint8_t size, bool hexImmediateData = false) const;

    // Addressing Modes Memory Access
    uint8_t  GetByte(uint8_t mode, uint8_t reg, uint16_t& calcTime, BusFlags flags = BUS_NORMAL);
    uint16_t GetWord(uint8_t mode, uint8_t reg, uint16_t& calcTime, BusFlags flags = BUS_NORMAL);
    uint32_t GetLong(uint8_t mode, uint8_t reg, uint16_t& calcTime, BusFlags flags = BUS_NORMAL);

    void SetByte(uint8_t mode, uint8_t reg, uint16_t& calcTime, uint8_t  data, BusFlags flags = BUS_NORMAL);
    void SetWord(uint8_t mode, uint8_t reg, uint16_t& calcTime, uint16_t data, BusFlags flags = BUS_NORMAL);
    void SetLong(uint8_t mode, uint8_t reg, uint16_t& calcTime, uint32_t data, BusFlags flags = BUS_NORMAL);

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

    // Conditional Tests
    bool T() const;
    bool F() const;
    bool HI() const;
    bool LS() const;
    bool CC() const;
    bool CS() const;
    bool NE() const;
    bool EQ() const;
    bool VC() const;
    bool VS() const;
    bool PL() const;
    bool MI() const;
    bool GE() const;
    bool LT() const;
    bool GT() const;
    bool LE() const;
    bool (SCC68070::*ConditionalTests[16])() const = {
        &SCC68070::T,
        &SCC68070::F,
        &SCC68070::HI,
        &SCC68070::LS,
        &SCC68070::CC,
        &SCC68070::CS,
        &SCC68070::NE,
        &SCC68070::EQ,
        &SCC68070::VC,
        &SCC68070::VS,
        &SCC68070::PL,
        &SCC68070::MI,
        &SCC68070::GE,
        &SCC68070::LT,
        &SCC68070::GT,
        &SCC68070::LE,
    };
    const char* DisassembleConditionalCode(uint8_t cc) const;

    // Instruction Set
    void ResetOperation();
    typedef uint16_t    (SCC68070::*ILUTFunctionPointer)();
    typedef std::string (SCC68070::*DLUTFunctionPointer)(uint32_t) const;
    void GenerateInstructionSet();
    void GenerateInstructionOpcodes(const char* format, std::vector<std::vector<int>> values, ILUTFunctionPointer instFunc, DLUTFunctionPointer disFunc);

    std::unique_ptr<ILUTFunctionPointer[]> ILUT; // Instructions Look Up Table
    uint16_t UnknownInstruction();
    uint16_t ABCD();
    uint16_t ADD();
    uint16_t ADDA();
    uint16_t ADDI();
    uint16_t ADDQ();
    uint16_t ADDX();
    uint16_t AND();
    uint16_t ANDI();
    uint16_t ANDICCR();
    uint16_t ANDISR();
    uint16_t ASm();
    uint16_t ASr();
    uint16_t Bcc();
    uint16_t BCHG();
    uint16_t BCLR();
    uint16_t BRA();
    uint16_t BSET();
    uint16_t BSR();
    uint16_t BTST();
    uint16_t CHK();
    uint16_t CLR();
    uint16_t CMP();
    uint16_t CMPA();
    uint16_t CMPI();
    uint16_t CMPM();
    uint16_t DBcc();
    uint16_t DIVS();
    uint16_t DIVU();
    uint16_t EOR();
    uint16_t EORI();
    uint16_t EORICCR();
    uint16_t EORISR();
    uint16_t EXG();
    uint16_t EXT();
    uint16_t ILLEGAL();
    uint16_t JMP();
    uint16_t JSR();
    uint16_t LEA();
    uint16_t LINK();
    uint16_t LSm();
    uint16_t LSr();
    uint16_t MOVE();
    uint16_t MOVEA();
    uint16_t MOVECCR();
    uint16_t MOVEfSR();
    uint16_t MOVESR();
    uint16_t MOVEUSP();
    uint16_t MOVEM();
    uint16_t MOVEP();
    uint16_t MOVEQ();
    uint16_t MULS();
    uint16_t MULU();
    uint16_t NBCD();
    uint16_t NEG();
    uint16_t NEGX();
    uint16_t NOP();
    uint16_t NOT();
    uint16_t OR();
    uint16_t ORI();
    uint16_t ORICCR();
    uint16_t ORISR();
    uint16_t PEA();
    uint16_t RESET();
    uint16_t ROm();
    uint16_t ROr();
    uint16_t ROXm();
    uint16_t ROXr();
    uint16_t RTE();
    uint16_t RTR();
    uint16_t RTS();
    uint16_t SBCD();
    uint16_t Scc();
    uint16_t STOP();
    uint16_t SUB();
    uint16_t SUBA();
    uint16_t SUBI();
    uint16_t SUBQ();
    uint16_t SUBX();
    uint16_t SWAP();
    uint16_t TAS();
    uint16_t TRAP();
    uint16_t TRAPV();
    uint16_t TST();
    uint16_t UNLK();

    std::unique_ptr<DLUTFunctionPointer[]> DLUT; // Disassembler Look Up Table
    std::string DisassembleUnknownInstruction(const uint32_t pc) const;
    std::string DisassembleABCD(const uint32_t pc) const;
    std::string DisassembleADD(const uint32_t pc) const;
    std::string DisassembleADDA(const uint32_t pc) const;
    std::string DisassembleADDI(const uint32_t pc) const;
    std::string DisassembleADDQ(const uint32_t pc) const;
    std::string DisassembleADDX(const uint32_t pc) const;
    std::string DisassembleAND(const uint32_t pc) const;
    std::string DisassembleANDI(const uint32_t pc) const;
    std::string DisassembleANDICCR(const uint32_t pc) const;
    std::string DisassembleANDISR(const uint32_t pc) const;
    std::string DisassembleASm(const uint32_t pc) const;
    std::string DisassembleASr(const uint32_t pc) const;
    std::string DisassembleBcc(const uint32_t pc) const;
    std::string DisassembleBCHG(const uint32_t pc) const;
    std::string DisassembleBCLR(const uint32_t pc) const;
    std::string DisassembleBRA(const uint32_t pc) const;
    std::string DisassembleBSET(const uint32_t pc) const;
    std::string DisassembleBSR(const uint32_t pc) const;
    std::string DisassembleBTST(const uint32_t pc) const;
    std::string DisassembleCHK(const uint32_t pc) const;
    std::string DisassembleCLR(const uint32_t pc) const;
    std::string DisassembleCMP(const uint32_t pc) const;
    std::string DisassembleCMPA(const uint32_t pc) const;
    std::string DisassembleCMPI(const uint32_t pc) const;
    std::string DisassembleCMPM(const uint32_t pc) const;
    std::string DisassembleDBcc(const uint32_t pc) const;
    std::string DisassembleDIVS(const uint32_t pc) const;
    std::string DisassembleDIVU(const uint32_t pc) const;
    std::string DisassembleEOR(const uint32_t pc) const;
    std::string DisassembleEORI(const uint32_t pc) const;
    std::string DisassembleEORICCR(const uint32_t pc) const;
    std::string DisassembleEORISR(const uint32_t pc) const;
    std::string DisassembleEXG(const uint32_t pc) const;
    std::string DisassembleEXT(const uint32_t pc) const;
    std::string DisassembleILLEGAL(const uint32_t pc) const;
    std::string DisassembleJMP(const uint32_t pc) const;
    std::string DisassembleJSR(const uint32_t pc) const;
    std::string DisassembleLEA(const uint32_t pc) const;
    std::string DisassembleLINK(const uint32_t pc) const;
    std::string DisassembleLSm(const uint32_t pc) const;
    std::string DisassembleLSr(const uint32_t pc) const;
    std::string DisassembleMOVE(const uint32_t pc) const;
    std::string DisassembleMOVEA(const uint32_t pc) const;
    std::string DisassembleMOVECCR(const uint32_t pc) const;
    std::string DisassembleMOVEfSR(const uint32_t pc) const;
    std::string DisassembleMOVESR(const uint32_t pc) const;
    std::string DisassembleMOVEUSP(const uint32_t pc) const;
    std::string DisassembleMOVEM(const uint32_t pc) const;
    std::string DisassembleMOVEP(const uint32_t pc) const;
    std::string DisassembleMOVEQ(const uint32_t pc) const;
    std::string DisassembleMULS(const uint32_t pc) const;
    std::string DisassembleMULU(const uint32_t pc) const;
    std::string DisassembleNBCD(const uint32_t pc) const;
    std::string DisassembleNEG(const uint32_t pc) const;
    std::string DisassembleNEGX(const uint32_t pc) const;
    std::string DisassembleNOP(const uint32_t pc) const;
    std::string DisassembleNOT(const uint32_t pc) const;
    std::string DisassembleOR(const uint32_t pc) const;
    std::string DisassembleORI(const uint32_t pc) const;
    std::string DisassembleORICCR(const uint32_t pc) const;
    std::string DisassembleORISR(const uint32_t pc) const;
    std::string DisassemblePEA(const uint32_t pc) const;
    std::string DisassembleRESET(const uint32_t pc) const;
    std::string DisassembleROm(const uint32_t pc) const;
    std::string DisassembleROr(const uint32_t pc) const;
    std::string DisassembleROXm(const uint32_t pc) const;
    std::string DisassembleROXr(const uint32_t pc) const;
    std::string DisassembleRTE(const uint32_t pc) const;
    std::string DisassembleRTR(const uint32_t pc) const;
    std::string DisassembleRTS(const uint32_t pc) const;
    std::string DisassembleSBCD(const uint32_t pc) const;
    std::string DisassembleScc(const uint32_t pc) const;
    std::string DisassembleSTOP(const uint32_t pc) const;
    std::string DisassembleSUB(const uint32_t pc) const;
    std::string DisassembleSUBA(const uint32_t pc) const;
    std::string DisassembleSUBI(const uint32_t pc) const;
    std::string DisassembleSUBQ(const uint32_t pc) const;
    std::string DisassembleSUBX(const uint32_t pc) const;
    std::string DisassembleSWAP(const uint32_t pc) const;
    std::string DisassembleTAS(const uint32_t pc) const;
    std::string DisassembleTRAP(const uint32_t pc) const;
    std::string DisassembleTRAPV(const uint32_t pc) const;
    std::string DisassembleTST(const uint32_t pc) const;
    std::string DisassembleUNLK(const uint32_t pc) const;
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

#define SR_UPPER_MASK (0xA700)

/* shorts for addressing modes */

#define ARIWPo(register, sizeInByte) AddressRegisterIndirectWithPostincrement(register, sizeInByte)
#define ARIWPr(register, sizeInByte) AddressRegisterIndirectWithPredecrement(register, sizeInByte)
#define ARIWD(register) AddressRegisterIndirectWithDisplacement(register)
#define ARIWI8(register) AddressRegisterIndirectWithIndex8(register)

#define PCIWD() ProgramCounterIndirectWithDisplacement()
#define PCIWI8() ProgramCounterIndirectWithIndex8()

#define ASA() AbsoluteShortAddressing()
#define ALA() AbsoluteLongAddressing()

// IT(Instruction Timing) + Addressing mode macro + size
#define ITREG 0 // register
#define ITARIBW 4 // Address Register Indirect (ARI)
#define ITARIL 8

#define ITARIWPoBW 4 // ARI with postincrement
#define ITARIWPoL 8
#define ITARIWPrBW 7 // ARI with predecrement
#define ITARIWPrL 11

#define ITARIWDBW 11 // ARI with displacement
#define ITARIWDL 15
#define ITARIWI8BW 14 // ARI with index 8
#define ITARIWI8L 18

#define ITPCIWDBW 11 // PC with displacement
#define ITPCIWDL 15
#define ITPCIWI8BW 14 // PC with index 8
#define ITPCIWI8L 18

#define ITASBW 8 // Absolute short
#define ITASL 12
#define ITALBW 12 // Absolute long
#define ITALL 16

#define ITIBW 4 // immediate
#define ITIL 8

#endif // CDI_CORES_SCC68070_SCC68070_HPP
