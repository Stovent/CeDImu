#ifndef CDI_CORES_MC68HC05_MC68HC05_HPP
#define CDI_CORES_MC68HC05_MC68HC05_HPP

#include <bitset>
#include <cstdint>

/** @class MC68HC05
 * @brief Implements the CPU core of a MC68HC05 microcontroller.
 */
class MC68HC05
{
protected:
    MC68HC05() = delete;
    MC68HC05(const MC68HC05&) = delete;
    explicit MC68HC05(uint16_t memorysize) : memorySize(memorysize), irqPin(true), waitStop(false), A(0), X(0), SP(0xFF), PC(0), CCR(0b1110'0000) {}
    virtual ~MC68HC05() {}

    const uint16_t memorySize;
    bool irqPin; /**< Physical state of the IRQ pin. Has to be set by the derived class. */
    bool waitStop; /**< true if a STOP or WAIT instruction is executed. Reseted to false by a RESET or IRQ (WAIT and STOP), or interrupts (WAIT). */

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
    virtual void Stop() = 0;
    /** @brief Called by the interpreter when a WAIT instruction is executed. */
    virtual void Wait() = 0;

    size_t Interpreter();

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

#endif // CDI_CORES_MC68HC05_MC68HC05_HPP
