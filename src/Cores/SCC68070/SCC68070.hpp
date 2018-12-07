#ifndef SCC68070_HPP
#define SCC68070_HPP

#include <cstdint>

#define UNCHANGED 2
#define OPCODESNBR 79
#define MEMORYSIZE 1048576

class SCC68070;

enum SCC68070InstructionSet
{
    UNKNOWN = 0,
    ABCD, // Add Decimal with Extend
    ADD, // Add Binary
    ADDA, // Add Address
    ADDI, // Add Immediate
    ADDQ, // Add Quick
    ADDX, // Add Extended
    AND, // Logical AND
    ANDI, // Logical AND Immediate
    ASm, // Arithmetic Shift Memory
    ASr, // Arithmetic Shift Register
    Bcc, // Branch Conditionally
    BCHGd, // Test Bit and Change Dynamic
    BCHGs, // Test Bit and Change Static
    BCLR, // Test Bit and Clear
    BRA, // Branch Always
    BSET, // Test a Bit and Set
    BSR, // Branch to Subroutine
    BTSTd,  // Test a Bit Dynamic
    BTSTs,  // Test a Bit Static
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
    EXG, // Exchange Register
    EXT, // Sign Extend (EXTB)
    JMP, // Jump  Branch
    JSR, // Jump to Subroutine  Branch
    LEA, // Load Effective Address
    LINK, // Link and Allocate
    LSm, // Logical Shift Memory
    LSr, // Logical Shift Register
    MOVE, // Move Data from Source to Destination
    MOVECCR, // Move to Condition Code Register
    MOVESR, // Move to the Status Register
    MOVEfSR, // Move from the Status Register
    MOVEUSP, // Move User Stack Pointer
    MOVEA, // Move Address
    MOVEM, // Move Multiple Registers
    MOVEP, // Move Peripheral Data
    MOVEQ, // Move Quick
    MULS, // Signed Multiply
    MULU, // Unsigned Multiply
    NBCD, // Negate Decimal with Extend
    NEG, // Negate
    NEGX, // Negate with Extend
    NOP, // No Operation Branch
    NOT, // Logical Complement
    OR, // Logical Inclusive-OR
    ORI, // Logical Inclusive-OR Immediate
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

class SCC68070
{
public:
    bool run;

    SCC68070();
    void Run();

private:
    int32_t D[8];
    int32_t A[8];
    uint32_t PC;
    uint16_t SR;

    uint16_t currentOpcode;
    uint32_t lastAddress;

    unsigned int executionTime;

    void (SCC68070::*Execute)() = nullptr;
    void Interpreter();
    uint16_t GetInstructionIf(const uint16_t& opcode);
    uint16_t GetNextOpcode();

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

    void Exception(const uint8_t& vec);

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
        T,
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

    // Instruction Set
    uint16_t UnknownInstruction();
    uint16_t Abcd();
    uint16_t Add();
    uint16_t Adda();
    uint16_t Addi();
    uint16_t Addq();
    uint16_t Addx();
    uint16_t And();
    uint16_t Andi();
    uint16_t AsM();
    uint16_t AsR();
    uint16_t BCC();
    uint16_t BchgD();
    uint16_t BchgS();
    uint16_t Bclr();
    uint16_t Bra();
    uint16_t Bset();
    uint16_t Bsr();
    uint16_t BtstD();
    uint16_t BtstS();
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
    uint16_t Exg();
    uint16_t Ext();
    uint16_t Jmp();
    uint16_t Jsr();
    uint16_t Lea();
    uint16_t Link();
    uint16_t LsM();
    uint16_t LsR();
    uint16_t Move();
    uint16_t Moveccr();
    uint16_t Movesr();
    uint16_t MoveFsr();
    uint16_t Moveusp();
    uint16_t Movea();
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
    uint16_t (SCC68070::*instructions[OPCODESNBR+1])() = {
        UnknownInstruction,
        Abcd,
        Add,
        Adda,
        Addi,
        Addq,
        Addx,
        And,
        Andi,
        AsM,
        AsR,
        BCC,
        BchgD,
        BchgS,
        Bclr,
        Bra,
        Bset,
        Bsr,
        BtstD,
        BtstS,
        Chk,
        Clr,
        Cmp,
        Cmpa,
        Cmpi,
        Cmpm,
        DbCC,
        Divs,
        Divu,
        Eor,
        Eori,
        Exg,
        Ext,
        Jmp,
        Jsr,
        Lea,
        Link,
        LsM,
        LsR,
        Move,
        Moveccr,
        Movesr,
        MoveFsr,
        Moveusp,
        Movea,
        Movem,
        Movep,
        Moveq,
        Muls,
        Mulu,
        Nbcd,
        Neg,
        Negx,
        Nop,
        Not,
        Or,
        Ori,
        Pea,
        Reset,
        RoM,
        RoR,
        RoxM,
        RoxR,
        Rte,
        Rtr,
        Rts,
        Sbcd,
        SCC,
        Stop,
        Sub,
        Suba,
        Subi,
        Subq,
        Subx,
        Swap,
        Tas,
        Trap,
        Trapv,
        Tst,
        Unlk
    };
};

#define GetNextWord() GetNextOpcode()
#define SP (A[7])

/* shorts for addressing modes */

#define ARIWPo(register, sizeInByte) AddressRegisterIndirectWithPostincrement(register, sizeInByte)
#define ARIWPr(register, sizeInByte) AddressRegisterIndirectWithPredecrement(register, sizeInByte)
#define ARIWD(register) AddressRegisterIndirectWithDisplacement(register)
#define ARIWI8(register) AddressRegisterIndirectWithIndex8(register)

#define PCIWD() ProgramCounterIndirectWithDisplacement()
#define PCIWI8() ProgramCounterIndirectWithIndex8()

#define AM7(register) AddressingMode7(register)

#define ASA() AbsoluteShortAddressing()
#define ALA() AbsoluteLongAddressing()

#endif // SCC68070_HPP
