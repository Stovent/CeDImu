#ifndef SCC68070_HPP
#define SCC68070_HPP

class SCC68070;

#include <cstdint>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include "../../common/flags.hpp"
#include "../VDSC.hpp"
#include "../../utils.hpp"

#define OPCODESNBR 84

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
    UNLK // Unlink
};

enum CC // Conditional Tests
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
    LE
};

enum ExceptionVectors
{
    ResetSSP = 0,
    ResetPC,
    BusError,
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

enum SCC68070Peripherals
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

    DMA = 0x80004000 - Base,

    MSR = 0x80008000 - Base, // MMU Status Register
    MCR = 0x80008001 - Base, // MMU Control Register
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

class SCC68070
{
public:
    bool disassemble;
    uint32_t currentPC;
    uint64_t totalCycleCount;

    std::vector<std::string> disassembledInstructions;

    SCC68070() = delete;
    SCC68070(SCC68070&) = delete;
    SCC68070(SCC68070&&) = delete;
    explicit SCC68070(VDSC* gpu, const uint32_t clockFrequency = 15500000L);
    ~SCC68070();

    bool IsRunning();
    void Run(const bool loop = true);
    void Stop();
    void Reset();

    void SetRegister(CPURegisters reg, const uint32_t value);
    std::map<std::string, uint32_t> GetRegisters();

private:
    VDSC* vdsc;
    std::thread executionThread;
    bool loop;
    bool isRunning;

    std::ofstream out;
    std::ofstream instruction;
    std::ofstream uart_out;
    std::ifstream uart_in;

    uint8_t* internal;

    uint16_t currentOpcode;
    uint32_t lastAddress;

    uint32_t cycleCount;
    long double cycleDelay; // Time between two clock cycles in nanoseconds

    void (SCC68070::*Execute)() = nullptr;
    void Interpreter();
    uint16_t GetNextWord(const uint8_t flags = Log | Trigger);
    void ResetOperation();

    // Registers
    int32_t D[8];
    uint32_t A[8];
    uint32_t PC;
    uint16_t SR;
    uint32_t USP;
    uint32_t SSP;

    // Conditional Codes
    void SetXC(const bool XC = 1); // Set both X and C at the same time
    void SetVC(const bool VC = 1); // Set both V and C at the same time
    void SetX(const bool X = 1);
    bool GetX();
    void SetN(const bool N = 1);
    bool GetN();
    void SetZ(const bool Z = 1);
    bool GetZ();
    void SetV(const bool V = 1);
    bool GetV();
    void SetC(const bool C = 1);
    bool GetC();
    void SetS(const bool S = 1);
    bool GetS();

    uint16_t Exception(const uint8_t vectorNumber);
    std::string DisassembleException(const uint8_t vectorNumber);

    // Addressing Modes
    int32_t GetIndexRegister(const uint16_t bew);

    uint32_t AddressRegisterIndirectWithPostincrement(const uint8_t reg, const uint8_t sizeInByte);
    uint32_t AddressRegisterIndirectWithPredecrement(const uint8_t reg, const uint8_t sizeInByte);
    uint32_t AddressRegisterIndirectWithDisplacement(const uint8_t reg);
    uint32_t AddressRegisterIndirectWithIndex8(const uint8_t reg);

    uint32_t ProgramCounterIndirectWithDisplacement();
    uint32_t ProgramCounterIndirectWithIndex8();

    uint32_t AbsoluteShortAddressing();
    uint32_t AbsoluteLongAddressing();

    std::string DisassembleAddressingMode(const uint32_t extWordAddress, const uint8_t eamode, const uint8_t eareg, const uint8_t size, const bool hexImmediateData = false);

    // Addressing Modes Memory Access
    uint8_t GetByte(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint8_t flags = Log | Trigger);
    uint16_t GetWord(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint8_t flags = Log | Trigger);
    uint32_t GetLong(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint8_t flags = Log | Trigger);

    void SetByte(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint8_t data, const uint8_t flags = Log | Trigger);
    void SetWord(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint16_t data, const uint8_t flags = Log | Trigger);
    void SetLong(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint32_t data, const uint8_t flags = Log | Trigger);

    // Direct Memory Access
    uint8_t GetByte(const uint32_t addr, const uint8_t flags = Log | Trigger);
    uint16_t GetWord(const uint32_t addr, const uint8_t flags = Log | Trigger);
    uint32_t GetLong(const uint32_t addr, const uint8_t flags = Log | Trigger);

    void SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags = Log | Trigger);
    void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Log | Trigger);
    void SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags = Log | Trigger);

    // Peripherals
    uint8_t GetPeripheral(const uint32_t addr);
    void SetPeripheral(const uint32_t addr, const uint8_t data);
    uint8_t ReadUART();
    void WriteUART(const uint8_t data);

    // Conditional Codes
    bool T();
    bool F();
    bool HI();
    bool LS();
    bool CC();
    bool CS();
    bool NE();
    bool EQ();
    bool VC();
    bool VS();
    bool PL();
    bool MI();
    bool GE();
    bool LT();
    bool GT();
    bool LE();
    bool (SCC68070::*ConditionalTests[16])() = {
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
    std::string DisassembleConditionalCode(const uint8_t cc);

    // Instruction Set
    void GenerateInstructionSet();
    void GenerateInstructionOpcodes(const char* format, std::vector<std::vector<int>> values, uint16_t (SCC68070::*instFunc)(), void (SCC68070::*disFunc)(uint32_t));

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

    void (SCC68070::**DLUT)(uint32_t); // Disassembler Look Up Table
    void DisassembleUnknownInstruction(uint32_t pc);
    void DisassembleABCD(uint32_t pc);
    void DisassembleADD(uint32_t pc);
    void DisassembleADDA(uint32_t pc);
    void DisassembleADDI(uint32_t pc);
    void DisassembleADDQ(uint32_t pc);
    void DisassembleADDX(uint32_t pc);
    void DisassembleAND(uint32_t pc);
    void DisassembleANDI(uint32_t pc);
    void DisassembleANDICCR(uint32_t pc);
    void DisassembleANDISR(uint32_t pc);
    void DisassembleASm(uint32_t pc);
    void DisassembleASr(uint32_t pc);
    void DisassembleBcc(uint32_t pc);
    void DisassembleBCHG(uint32_t pc);
    void DisassembleBCLR(uint32_t pc);
    void DisassembleBRA(uint32_t pc);
    void DisassembleBSET(uint32_t pc);
    void DisassembleBSR(uint32_t pc);
    void DisassembleBTST(uint32_t pc);
    void DisassembleCHK(uint32_t pc);
    void DisassembleCLR(uint32_t pc);
    void DisassembleCMP(uint32_t pc);
    void DisassembleCMPA(uint32_t pc);
    void DisassembleCMPI(uint32_t pc);
    void DisassembleCMPM(uint32_t pc);
    void DisassembleDBcc(uint32_t pc);
    void DisassembleDIVS(uint32_t pc);
    void DisassembleDIVU(uint32_t pc);
    void DisassembleEOR(uint32_t pc);
    void DisassembleEORI(uint32_t pc);
    void DisassembleEORICCR(uint32_t pc);
    void DisassembleEORISR(uint32_t pc);
    void DisassembleEXG(uint32_t pc);
    void DisassembleEXT(uint32_t pc);
    void DisassembleILLEGAL(uint32_t pc);
    void DisassembleJMP(uint32_t pc);
    void DisassembleJSR(uint32_t pc);
    void DisassembleLEA(uint32_t pc);
    void DisassembleLINK(uint32_t pc);
    void DisassembleLSm(uint32_t pc);
    void DisassembleLSr(uint32_t pc);
    void DisassembleMOVE(uint32_t pc);
    void DisassembleMOVEA(uint32_t pc);
    void DisassembleMOVECCR(uint32_t pc);
    void DisassembleMOVEfSR(uint32_t pc);
    void DisassembleMOVESR(uint32_t pc);
    void DisassembleMOVEUSP(uint32_t pc);
    void DisassembleMOVEM(uint32_t pc);
    void DisassembleMOVEP(uint32_t pc);
    void DisassembleMOVEQ(uint32_t pc);
    void DisassembleMULS(uint32_t pc);
    void DisassembleMULU(uint32_t pc);
    void DisassembleNBCD(uint32_t pc);
    void DisassembleNEG(uint32_t pc);
    void DisassembleNEGX(uint32_t pc);
    void DisassembleNOP(uint32_t pc);
    void DisassembleNOT(uint32_t pc);
    void DisassembleOR(uint32_t pc);
    void DisassembleORI(uint32_t pc);
    void DisassembleORICCR(uint32_t pc);
    void DisassembleORISR(uint32_t pc);
    void DisassemblePEA(uint32_t pc);
    void DisassembleRESET(uint32_t pc);
    void DisassembleROm(uint32_t pc);
    void DisassembleROr(uint32_t pc);
    void DisassembleROXm(uint32_t pc);
    void DisassembleROXr(uint32_t pc);
    void DisassembleRTE(uint32_t pc);
    void DisassembleRTR(uint32_t pc);
    void DisassembleRTS(uint32_t pc);
    void DisassembleSBCD(uint32_t pc);
    void DisassembleScc(uint32_t pc);
    void DisassembleSTOP(uint32_t pc);
    void DisassembleSUB(uint32_t pc);
    void DisassembleSUBA(uint32_t pc);
    void DisassembleSUBI(uint32_t pc);
    void DisassembleSUBQ(uint32_t pc);
    void DisassembleSUBX(uint32_t pc);
    void DisassembleSWAP(uint32_t pc);
    void DisassembleTAS(uint32_t pc);
    void DisassembleTRAP(uint32_t pc);
    void DisassembleTRAPV(uint32_t pc);
    void DisassembleTST(uint32_t pc);
    void DisassembleUNLK(uint32_t pc);
};

typedef uint16_t (SCC68070::*ILUTFunctionPointer)();
typedef void     (SCC68070::*DLUTFunctionPointer)(uint32_t);

#define SET_TX_READY internal[USR] |= 0x04;
#define SET_RX_READY internal[USR] |= 0x01;

/* shorts for addressing modes */

#define ARIWPo(register, sizeInByte) AddressRegisterIndirectWithPostincrement(register, sizeInByte)
#define ARIWPr(register, sizeInByte) AddressRegisterIndirectWithPredecrement(register, sizeInByte)
#define ARIWD(register) AddressRegisterIndirectWithDisplacement(register)
#define ARIWI8(register) AddressRegisterIndirectWithIndex8(register)

#define PCIWD() ProgramCounterIndirectWithDisplacement()
#define PCIWI8() ProgramCounterIndirectWithIndex8()

//#define AM7(register) AddressingMode7(register)

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
