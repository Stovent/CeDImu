#ifndef SCC68070_HPP
#define SCC68070_HPP

class SCC68070;

#include <cstdint>
#include <fstream>
#include <vector>
#include <string>

#include "../VDSC.hpp"
#include "../../utils.hpp"

#define UNCHANGED 2
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
    Last = 0x8000807F,
    Size = Last - Base + 1,

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

class SCC68070
{
public:
    int32_t D[8];
    uint32_t A[8];
    uint32_t PC;
    uint16_t SR;
    uint32_t USP;
    uint32_t SSP;

    uint32_t currentPC;
    bool run;
    bool disassemble;
    uint64_t totalCycleCount;

    std::vector<std::string> disassembledInstructions;

    SCC68070() = delete;
    SCC68070(SCC68070&) = delete;
    SCC68070(SCC68070&&) = delete;
    explicit SCC68070(VDSC* gpu, const uint32_t clockFrequency = 15500000L);
    ~SCC68070();

    void Run();
    void RebootCore();
    void SingleStep();
    void ResetOperation();
    uint64_t instructionCount;

private:
    VDSC* vdsc;

    std::ofstream out;
    std::ofstream instruction;
    std::ofstream uart_out;
    std::ifstream uart_in;

    uint8_t* internal;

    uint16_t currentOpcode;
    uint32_t lastAddress;

    uint32_t cycleCount;
    long double cycleDelay; // Time between two clock cycles in nanoseconds

    void (SCC68070::*Execute)(const bool loop) = nullptr;
    void Interpreter(const bool loop);
    uint16_t GetNextWord();

    // Conditional Codes
    void SetCCR(const uint8_t X, const uint8_t N, const uint8_t Z, const uint8_t V, const uint8_t C);
    void SetXC(const uint8_t XC = 1); // Set both X and C at the same time
    void SetVC(const uint8_t VC = 1); // Set both V and C at the same time
    void SetX(const uint8_t X = 1);
    uint8_t GetX();
    void SetN(const uint8_t N = 1);
    uint8_t GetN();
    void SetZ(const uint8_t Z = 1);
    uint8_t GetZ();
    void SetV(const uint8_t V = 1);
    uint8_t GetV();
    void SetC(const uint8_t C = 1);
    uint8_t GetC();
    void SetS(const uint8_t S = 1);
    uint8_t GetS();

    uint16_t Exception(const uint8_t vectorNumber);
    std::string DisassembleException(const uint8_t vectorNumber);

    // Addressing modes
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

    // Addressing modes memory access
    uint8_t GetByte(const uint8_t mode, const uint8_t reg, uint16_t& calcTime);
    uint16_t GetWord(const uint8_t mode, const uint8_t reg, uint16_t& calcTime);
    uint32_t GetLong(const uint8_t mode, const uint8_t reg, uint16_t& calcTime);

    void SetByte(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint8_t data);
    void SetWord(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint16_t data);
    void SetLong(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint32_t data);

    // Direct Memory Access
    uint8_t GetByte(const uint32_t addr);
    uint16_t GetWord(const uint32_t addr);
    uint32_t GetLong(const uint32_t addr);

    void SetByte(const uint32_t addr, const uint8_t data);
    void SetWord(const uint32_t addr, const uint16_t data);
    void SetLong(const uint32_t addr, const uint32_t data);

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
    std::string DisassembleConditionalCode(uint8_t cc);

    // Instruction Set
    void GenerateInstructionSet();
    void GenerateInstructionOpcodes(const char* format, std::vector<std::vector<int>> values, uint16_t (SCC68070::*instFunc)(), void (SCC68070::*disFunc)(uint32_t));

    uint16_t (SCC68070::*ILUT[UINT16_MAX+1])(); // Instructions Look Up Table
    uint16_t UnknownInstruction();
    uint16_t Abcd();
    uint16_t Add();
    uint16_t Adda();
    uint16_t Addi();
    uint16_t Addq();
    uint16_t Addx();
    uint16_t And();
    uint16_t Andi();
    uint16_t Andiccr();
    uint16_t Andisr();
    uint16_t AsM();
    uint16_t AsR();
    uint16_t BCC();
    uint16_t Bchg();
    uint16_t Bclr();
    uint16_t Bra();
    uint16_t Bset();
    uint16_t Bsr();
    uint16_t Btst();
    uint16_t Chk();
    uint16_t Clr();
    uint16_t Cmp();
    uint16_t Cmpa();
    uint16_t Cmpi();
    uint16_t Cmpm();
    uint16_t DbCC();
    uint16_t Divs();
    uint16_t Divu();
    uint16_t Eor();
    uint16_t Eori();
    uint16_t Eoriccr();
    uint16_t Eorisr();
    uint16_t Exg();
    uint16_t Ext();
    uint16_t Illegal();
    uint16_t Jmp();
    uint16_t Jsr();
    uint16_t Lea();
    uint16_t Link();
    uint16_t LsM();
    uint16_t LsR();
    uint16_t Move();
    uint16_t Movea();
    uint16_t Moveccr();
    uint16_t MoveFsr();
    uint16_t Movesr();
    uint16_t Moveusp();
    uint16_t Movem();
    uint16_t Movep();
    uint16_t Moveq();
    uint16_t Muls();
    uint16_t Mulu();
    uint16_t Nbcd();
    uint16_t Neg();
    uint16_t Negx();
    uint16_t Nop();
    uint16_t Not();
    uint16_t Or();
    uint16_t Ori();
    uint16_t Oriccr();
    uint16_t Orisr();
    uint16_t Pea();
    uint16_t Reset();
    uint16_t RoM();
    uint16_t RoR();
    uint16_t RoxM();
    uint16_t RoxR();
    uint16_t Rte();
    uint16_t Rtr();
    uint16_t Rts();
    uint16_t Sbcd();
    uint16_t SCC();
    uint16_t Stop();
    uint16_t Sub();
    uint16_t Suba();
    uint16_t Subi();
    uint16_t Subq();
    uint16_t Subx();
    uint16_t Swap();
    uint16_t Tas();
    uint16_t Trap();
    uint16_t Trapv();
    uint16_t Tst();
    uint16_t Unlk();

    void (SCC68070::*DLUT[UINT16_MAX+1])(uint32_t); // Disassembler Look Up Table
    void DisassembleUnknownInstruction(uint32_t pc);
    void DisassembleAbcd(uint32_t pc);
    void DisassembleAdd(uint32_t pc);
    void DisassembleAdda(uint32_t pc);
    void DisassembleAddi(uint32_t pc);
    void DisassembleAddq(uint32_t pc);
    void DisassembleAddx(uint32_t pc);
    void DisassembleAnd(uint32_t pc);
    void DisassembleAndi(uint32_t pc);
    void DisassembleAndiccr(uint32_t pc);
    void DisassembleAndisr(uint32_t pc);
    void DisassembleAsM(uint32_t pc);
    void DisassembleAsR(uint32_t pc);
    void DisassembleBCC(uint32_t pc);
    void DisassembleBchg(uint32_t pc);
    void DisassembleBclr(uint32_t pc);
    void DisassembleBra(uint32_t pc);
    void DisassembleBset(uint32_t pc);
    void DisassembleBsr(uint32_t pc);
    void DisassembleBtst(uint32_t pc);
    void DisassembleChk(uint32_t pc);
    void DisassembleClr(uint32_t pc);
    void DisassembleCmp(uint32_t pc);
    void DisassembleCmpa(uint32_t pc);
    void DisassembleCmpi(uint32_t pc);
    void DisassembleCmpm(uint32_t pc);
    void DisassembleDbCC(uint32_t pc);
    void DisassembleDivs(uint32_t pc);
    void DisassembleDivu(uint32_t pc);
    void DisassembleEor(uint32_t pc);
    void DisassembleEori(uint32_t pc);
    void DisassembleEoriccr(uint32_t pc);
    void DisassembleEorisr(uint32_t pc);
    void DisassembleExg(uint32_t pc);
    void DisassembleExt(uint32_t pc);
    void DisassembleIllegal(uint32_t pc);
    void DisassembleJmp(uint32_t pc);
    void DisassembleJsr(uint32_t pc);
    void DisassembleLea(uint32_t pc);
    void DisassembleLink(uint32_t pc);
    void DisassembleLsM(uint32_t pc);
    void DisassembleLsR(uint32_t pc);
    void DisassembleMove(uint32_t pc);
    void DisassembleMovea(uint32_t pc);
    void DisassembleMoveccr(uint32_t pc);
    void DisassembleMoveFsr(uint32_t pc);
    void DisassembleMovesr(uint32_t pc);
    void DisassembleMoveusp(uint32_t pc);
    void DisassembleMovem(uint32_t pc);
    void DisassembleMovep(uint32_t pc);
    void DisassembleMoveq(uint32_t pc);
    void DisassembleMuls(uint32_t pc);
    void DisassembleMulu(uint32_t pc);
    void DisassembleNbcd(uint32_t pc);
    void DisassembleNeg(uint32_t pc);
    void DisassembleNegx(uint32_t pc);
    void DisassembleNop(uint32_t pc);
    void DisassembleNot(uint32_t pc);
    void DisassembleOr(uint32_t pc);
    void DisassembleOri(uint32_t pc);
    void DisassembleOriccr(uint32_t pc);
    void DisassembleOrisr(uint32_t pc);
    void DisassemblePea(uint32_t pc);
    void DisassembleReset(uint32_t pc);
    void DisassembleRoM(uint32_t pc);
    void DisassembleRoR(uint32_t pc);
    void DisassembleRoxM(uint32_t pc);
    void DisassembleRoxR(uint32_t pc);
    void DisassembleRte(uint32_t pc);
    void DisassembleRtr(uint32_t pc);
    void DisassembleRts(uint32_t pc);
    void DisassembleSbcd(uint32_t pc);
    void DisassembleSCC(uint32_t pc);
    void DisassembleStop(uint32_t pc);
    void DisassembleSub(uint32_t pc);
    void DisassembleSuba(uint32_t pc);
    void DisassembleSubi(uint32_t pc);
    void DisassembleSubq(uint32_t pc);
    void DisassembleSubx(uint32_t pc);
    void DisassembleSwap(uint32_t pc);
    void DisassembleTas(uint32_t pc);
    void DisassembleTrap(uint32_t pc);
    void DisassembleTrapv(uint32_t pc);
    void DisassembleTst(uint32_t pc);
    void DisassembleUnlk(uint32_t pc);
};

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
