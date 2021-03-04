#ifndef SCC68070_HPP
#define SCC68070_HPP

class Board;
class SCC68070;
class SCC68070Exception;
#include "../../common/flags.hpp"

#include <cstdint>
#include <cstdio>
#include <functional>
#include <map>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#define SCC68070_DEFAULT_FREQUENCY (15500000L)

// Actually, figure VI.1 of the Green Book
// and Table 2-2 of the MC68000UM
// contains more information than the SCC68070 datasheet itself
enum SCC68070InstructionSet
{
    UNKNOWN_INSTRUCTION = 0,
    ABCD, // Add Decimal with Extend
    ADD, // Add Binary
    ADDA, // Add Address
    ADDI, // Add Immediate
    ADDQ, // Add Quick
    ADDX, // Add Extended
    AND, // Logical AND
    ANDI, // Logical AND Immediate
    ANDICCR, // CCR AND Immediate
    ANDISR, // AND Immediate to the Status Register
    ASm, // Arithmetic Shift Memory
    ASr, // Arithmetic Shift Register
    Bcc, // Branch Conditionally
    BCHG, // Test Bit and Change
    BCLR, // Test Bit and Clear
    BRA, // Branch Always
    BSET, // Test a Bit and Set
    BSR, // Branch to Subroutine
    BTST,  // Test a Bit
    CHK, // Check Register Against Bound
    CLR, // Clear an Operand
    CMP, // Compare
    CMPA, // Compare Address
    CMPI, // Compare Immediate
    CMPM, // Compare Memory
    DBcc, // Test Condition, Decrement and Branch
    DIVS, // Signed Divide
    DIVU, // Unsigned Divide
    EOR, // Logical Exclusive-OR
    EORI, // Logical Exclusive-OR Immediate
    EORICCR, // Exclusive-OR Immediateto to Condition Code
    EORISR, // Exclusive-OR Immediate to the Status Register
    EXG, // Exchange Register
    EXT, // Sign Extend (EXTB)
    ILLEGAL, // Take Illegal Instruction Trap
    JMP, // Jump Branch
    JSR, // Jump to Subroutine  Branch
    LEA, // Load Effective Address
    LINK, // Link and Allocate
    LSm, // Logical Shift Memory
    LSr, // Logical Shift Register
    MOVE, // Move Data from Source to Destination
    MOVEA, // Move Address
    MOVECCR, // Move to Condition Code Register
    MOVEfSR, // Move from the Status Register
    MOVESR, // Move to the Status Register
    MOVEUSP, // Move User Stack Pointer
    MOVEM, // Move Multiple Registers
    MOVEP, // Move Peripheral Data
    MOVEQ, // Move Quick
    MULS, // Signed Multiply
    MULU, // Unsigned Multiply
    NBCD, // Negate Decimal with Extend
    NEG, // Negate
    NEGX, // Negate with Extend
    NOP, // No Operation
    NOT, // Logical Complement
    OR, // Logical Inclusive-OR
    ORI, // Logical Inclusive-OR Immediate
    ORICCR, // Inclusive-OR Immediate to Condition Codes
    ORISR, // Inclusive-OR Immediate to the Status Register
    PEA, // Push Effective Address
    RESET, // Reset External Devices
    ROm, // Rotate without Extend Memory
    ROr, // Rotate without Extend Register
    ROXm, // Rotate with Extend Memory
    ROXr, // Rotate with Extend Register
    RTE, // Return from Exception
    RTR, // Return and Restore Condition Codes
    RTS, // Return from Subroutine
    SBCD, // Subtract Decimal with Extend
    Scc, // Set According to Condition Branch
    STOP, // Load Status Register and Stop
    SUB, // Subtract Binary
    SUBA, // Subtract Address
    SUBI, // Subtract Immediate
    SUBQ, // Subtract Quick
    SUBX, // Subtract with Extend
    SWAP, // Swap Register Halves
    TAS, // Test and Set Operand
    TRAP, // Trap
    TRAPV, // Trap on Overflow
    TST, // Test an Operand Branch
    UNLK, // Unlink
};

enum CC : uint8_t // Conditional Tests
{
    T = 0,
    F,
    HI,
    LS,
    CC,
    CS,
    NE,
    EQ,
    VC,
    VS,
    PL,
    MI,
    GE,
    LT,
    GT,
    LE,
};

enum ExceptionVectors : uint8_t
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
    Level1OnChipInterruptAutovector = 57,
    Level2OnChipInterruptAutovector,
    Level3OnChipInterruptAutovector,
    Level4OnChipInterruptAutovector,
    Level5OnChipInterruptAutovector,
    Level6OnChipInterruptAutovector,
    Level7OnChipInterruptAutovector,
};

enum SCC68070Peripherals : uint32_t
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
    CPR1   = 0x8000402B - Base, // Channel Priority Register 1

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
    CPR2   = 0x8000406B - Base, // Channel Priority Register 2

    MSR  = 0x80008000 - Base, // MMU Status Register
    MCR  = 0x80008001 - Base, // MMU Control Register
    ATTR = 0x80008040 - Base,
    SEG_LENGTH = 0x80008042 - Base,
    SEG_NUMBER = 0x80008045 - Base,
    BASE_ADDRESS = 0x80008046 - Base,
};

enum class CPURegisters : uint16_t
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

struct SCC68070Exception
{
    uint8_t vector;
    int8_t group;
    uint16_t data;
    SCC68070Exception(const uint8_t vec, const int8_t grp) : vector(vec), group(grp), data(0) {}
    SCC68070Exception(const uint8_t vec, const int8_t grp, const uint16_t d) : vector(vec), group(grp), data(d) {}
    SCC68070Exception(const SCC68070Exception&) = default;
};

bool operator>(const SCC68070Exception& lhs, const SCC68070Exception& rhs);

struct CPUInternalRegister
{
    std::string name;
    uint32_t address;
    uint16_t value;
    std::string disassembledValue;
};

class SCC68070
{
public:
    bool disassemble;
    uint32_t currentPC;
    uint64_t totalCycleCount;

    std::vector<std::string> disassembledInstructions;
    std::vector<uint32_t> breakpoints;

    std::function<void(uint8_t)> OnUARTOut;

    SCC68070() = delete;
    SCC68070(SCC68070&) = delete;
    SCC68070(SCC68070&&) = delete;
    explicit SCC68070(Board* baord, const uint32_t clockFrequency = SCC68070_DEFAULT_FREQUENCY);
    ~SCC68070();

    bool IsRunning() const;
    void SetEmulationSpeed(const double speed);
    void FlushDisassembler();

    void Run(const bool loop = true);
    void Stop(const bool wait = true);
    void Reset();

    void SetRegister(CPURegisters reg, const uint32_t value);
    std::map<std::string, uint32_t> GetCPURegisters() const;
    std::vector<CPUInternalRegister> GetInternalRegisters() const;

private:
    Board* board;
    std::thread executionThread;
    bool loop;
    bool isRunning;

    FILE* out;
    FILE* instructions;

    uint8_t* internal;

    bool flushDisassembler;
    uint16_t currentOpcode;
    uint32_t lastAddress;
    uint32_t cycleCount;
    const double cycleDelay; // Time between two clock cycles in nanoseconds
    const double timerDelay;
    double speedDelay; // used for emulation speed.
    double timerCounter; // Counts the nanosconds when incrementing the timer.
    std::priority_queue<SCC68070Exception, std::vector<SCC68070Exception>, std::greater<SCC68070Exception>> exceptions;

    void Interpreter();
    uint16_t GetNextWord(const uint8_t flags = Log | Trigger);
    uint16_t PeekNextWord();
    void ResetOperation();
    void DumpCPURegisters();

    // Registers
    uint32_t D[8];
    uint32_t A[8];
    uint32_t PC;
    uint16_t SR;
    uint32_t USP;
    uint32_t SSP;

    // Conditional Codes
    bool GetS() const;
    void SetS(const bool S = 1);
    bool GetX() const;
    void SetX(const bool X = 1);
    bool GetN() const;
    void SetN(const bool N = 1);
    bool GetZ() const;
    void SetZ(const bool Z = 1);
    bool GetV() const;
    void SetV(const bool V = 1);
    bool GetC() const;
    void SetC(const bool C = 1);
    void SetXC(const bool XC = 1); // Set both X and C at the same time
    void SetVC(const bool VC = 1); // Set both V and C at the same time

    uint16_t Exception(const uint8_t vectorNumber);
    std::string DisassembleException(const SCC68070Exception& exception) const;

    // Addressing Modes
    uint32_t GetEffectiveAddress(const uint8_t mode, const uint8_t reg, const uint8_t sizeInBytes, uint16_t& calcTime);
    int32_t GetIndexRegister(const uint16_t bew) const;

    uint32_t AddressRegisterIndirectWithPostincrement(const uint8_t reg, const uint8_t sizeInByte);
    uint32_t AddressRegisterIndirectWithPredecrement(const uint8_t reg, const uint8_t sizeInByte);
    uint32_t AddressRegisterIndirectWithDisplacement(const uint8_t reg);
    uint32_t AddressRegisterIndirectWithIndex8(const uint8_t reg);

    uint32_t ProgramCounterIndirectWithDisplacement();
    uint32_t ProgramCounterIndirectWithIndex8();

    uint32_t AbsoluteShortAddressing();
    uint32_t AbsoluteLongAddressing();

    std::string DisassembleAddressingMode(const uint32_t extWordAddress, const uint8_t eamode, const uint8_t eareg, const uint8_t size, const bool hexImmediateData = false) const;

    // Addressing Modes Memory Access
    uint8_t  GetByte(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint8_t flags = Log | Trigger);
    uint16_t GetWord(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint8_t flags = Log | Trigger);
    uint32_t GetLong(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint8_t flags = Log | Trigger);

    void SetByte(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint8_t  data, const uint8_t flags = Log | Trigger);
    void SetWord(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint16_t data, const uint8_t flags = Log | Trigger);
    void SetLong(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint32_t data, const uint8_t flags = Log | Trigger);

    // Direct Memory Access
    uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Log | Trigger);
    uint16_t GetWord(const uint32_t addr, const uint8_t flags = Log | Trigger);
    uint32_t GetLong(const uint32_t addr, const uint8_t flags = Log | Trigger);

    void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Log | Trigger);
    void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Log | Trigger);
    void SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags = Log | Trigger);

    // Peripherals
    uint8_t GetPeripheral(uint32_t addr);
    void SetPeripheral(uint32_t addr, const uint8_t data);
    void IncrementTimer(const double ns);

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
        &SCC68070::LE
    };
    std::string DisassembleConditionalCode(const uint8_t cc) const;

    // Instruction Set
    typedef uint16_t    (SCC68070::*ILUTFunctionPointer)();
    typedef std::string (SCC68070::*DLUTFunctionPointer)(const uint32_t) const;
    void GenerateInstructionSet();
    void GenerateInstructionOpcodes(const char* format, std::vector<std::vector<int>> values, ILUTFunctionPointer instFunc, DLUTFunctionPointer disFunc);

    uint16_t (SCC68070::**ILUT)(); // Instructions Look Up Table
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

    std::string (SCC68070::**DLUT)(const uint32_t) const; // Disassembler Look Up Table
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

#define   SET_TX_READY() internal[USR] |= 0x04;
#define   SET_RX_READY() internal[USR] |= 0x01;
#define UNSET_TX_READY() internal[USR] &= ~0x04;
#define UNSET_RX_READY() internal[USR] &= ~0x01;

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

#endif // SCC68070_HPP
