#ifndef CDI_CORES_MC68HC05_MC68HC05_HPP
#define CDI_CORES_MC68HC05_MC68HC05_HPP

#include <bitset>
#include <cstdint>

/** @class MC68HC05
 * @brief Implements the CPU core of a MC68HC05 microcontroller.
 */
class MC68HC05
{
public:
    static constexpr size_t EXTERNAL_FREQUENCY = 4'000'000;
    static constexpr size_t INTERNAL_BUS_FREQUENCY = EXTERNAL_FREQUENCY / 2;
    static constexpr size_t TIMER_FREQUENCY = INTERNAL_BUS_FREQUENCY / 4;

protected:
    enum CCRBits
    {
        CCRC = 0,
        CCRZ,
        CCRN,
        CCRI,
        CCRH,
    };

    MC68HC05() = delete;
    MC68HC05(const MC68HC05&) = delete;
    explicit MC68HC05(uint16_t memorysize) : memorySize(memorysize), irqPin(true), stop(false), wait(false), A(0), X(0), SP(0xFF), PC(0), CCR(0b1110'0000) {}
    virtual ~MC68HC05() {}

    const uint16_t memorySize;
    bool irqPin; /**< Physical state of the IRQ pin. Has to be set by the derived class. */
    bool stop; /**< true if a STOP instruction has been executed. Reseted to false by a RESET or IRQ. */
    bool wait; /**< true if a WAIT instruction has been executed. Reseted to false by a RESET, IRQ or interrupts. */

    uint8_t A;
    uint8_t X;
    uint16_t SP;
    uint16_t PC;
    std::bitset<8> CCR;

    /** @brief Reads a byte from internal memory. */
    virtual uint8_t GetMemory(uint16_t addr) = 0;
    /** @brief Writes a byte to internal memory. */
    virtual void SetMemory(uint16_t addr, uint8_t value) = 0;

    virtual void Reset();
    /** @brief Called by the interpreter when a STOP instruction is executed. */
    virtual void Stop() {};
    /** @brief Called by the interpreter when a WAIT instruction is executed. */
    virtual void Wait() {};

    size_t Interpreter();
    template<bool MASKABLE = true> void Interrupt(uint16_t addr);

private:
    // Memory Access
    void PushByte(uint8_t data);
    uint8_t PopByte();
    uint8_t GetNextByte();

    // Effective Address
    uint16_t EA_DIR();
    uint16_t EA_EXT();
    uint16_t EA_IX();
    uint16_t EA_IX1();
    uint16_t EA_IX2();

    // Operand Access
    uint8_t IMM();
    uint8_t DIR();
    uint8_t EXT();
    uint8_t IX();
    uint8_t IX1();
    uint8_t IX2();

    // Instruction Set
    void ADC(uint8_t rhs);
    void ADD(uint8_t rhs);
    void AND(uint8_t rhs);
    void ASR(uint8_t& reg);
    void ASR(uint16_t addr);
    template<int BIT> void BCLR();
    uint8_t BIT(uint8_t rhs);
    void Branch(bool condition);
    template<int BIT> void BRCLR();
    template<int BIT> void BRSET();
    template<int BIT> void BSET();
    void CLR(uint8_t& reg);
    void CLR(uint16_t addr);
    uint8_t CMP(uint8_t lhs, uint8_t rhs);
    void COM(uint8_t& reg);
    void COM(uint16_t addr);
    void DEC(uint8_t& reg);
    void DEC(uint16_t addr);
    void EOR(uint8_t rhs);
    void INC(uint8_t& reg);
    void INC(uint16_t addr);
    void JMP(uint16_t addr);
    void JSR(uint16_t addr);
    void LDAX(uint8_t& reg, uint8_t rhs);
    void LSL(uint8_t& reg);
    void LSL(uint16_t addr);
    void LSR(uint8_t& reg);
    void LSR(uint16_t addr);
    void NEG(uint8_t& reg);
    void NEG(uint16_t addr);
    void ORA(uint8_t rhs);
    void ROL(uint8_t& reg);
    void ROL(uint16_t addr);
    void ROR(uint8_t& reg);
    void ROR(uint16_t addr);
    void SBC(uint8_t rhs);
    void STAX(uint8_t val, uint16_t addr);
    void SUB(uint8_t rhs);
    void TST(uint8_t val);
};

// Old language...
/** @brief Generates an interrupt.
 *  @param addr The address where to fetch the high byte of the vector.
 *
 * If the interrupt is maskable and interrupts are disabled, then this function does nothing.
 */
template<bool MASKABLE = true>
void MC68HC05::Interrupt(uint16_t addr)
{
    if(MASKABLE && CCR[CCRI])
        return; // Maskable interrupts disabled.

    PushByte(PC);
    PushByte(PC >> 8);
    PushByte(X);
    PushByte(A);
    PushByte(CCR.to_ulong());
    CCR[CCRI] = true;
    PC = (uint16_t)GetMemory(addr) << 8;
    PC |= GetMemory(addr + 1);
}

#endif // CDI_CORES_MC68HC05_MC68HC05_HPP
