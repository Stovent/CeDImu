#ifndef MC68HC705C8_HPP
#define MC68HC705C8_HPP

class MC68HC705C8;

#include <bitset>
#include <cstdint>
#include <cstdio>
#include <string>

#define SLAVE_MEMORY_SIZE (0x2000)

enum CCRBits
{
    C = 0,
    Z,
    N,
    I,
    H,
};

class MC68HC705C8
{
public:
    MC68HC705C8() = delete;
    MC68HC705C8(const MC68HC705C8&) = delete;
    MC68HC705C8(const void* bios, uint16_t size);
    ~MC68HC705C8();

    void Reset();
    void Execute(const int cycles);
    void IRQ();

    uint8_t* GetMemory() const { return memory; }

private:
    uint8_t A;
    uint8_t X;
    uint16_t PC;
    uint8_t SP;
    std::bitset<8> CCR;

    uint8_t* memory;
    bool waitStop;
    bool irqPin; // true = high, false = low

    uint8_t currentOpcode;
    uint16_t currentPC;
    FILE* instructions;
    int pendingCycles;

    // Memory Access
    uint8_t GetByte(const uint16_t addr);
    void SetByte(const uint16_t addr, const uint8_t value);
    uint8_t GetNextByte();
    void PushByte(const uint8_t data);
    uint8_t PopByte();

    uint8_t IndirectThreadedCode();
};

#endif // MC68HC705C8_HPP
