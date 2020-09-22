#ifndef MC68HC705C8_HPP
#define MC68HC705C8_HPP

class MC68HC705C8;

#include <cstdint>
#include <fstream>
#include <string>

class MC68HC705C8
{
public:
    MC68HC705C8(const void* bios, uint16_t size);
    ~MC68HC705C8();

    void Reset();
    void Execute(const int cycles);

private:
    uint8_t A;
    uint8_t X;
    uint16_t PC;
    union {
        uint8_t byte;
        struct {
//            const uint8_t unused : 2 = 0b11;
            uint8_t unused : 2;
            uint8_t SP : 6;
        } ;
    } SP;
//    uint8_t SP; // this or upper?
    union {
        uint8_t byte;
        struct {
            : 3;
            uint8_t H : 1;
            uint8_t I : 1;
            uint8_t N : 1;
            uint8_t Z : 1;
            uint8_t C : 1;
        };
    } CCR;
//    uint8_t CCR; // this or the same thing as SP?

    uint8_t* memory;

    uint8_t currentOpcode;
    uint16_t currentPC;
    std::ofstream instructions;
    int pendingCycles;

    // Addressing Modes
    uint8_t GetByteIndexed();
    void SetByteIndexed(const uint8_t value);
    uint8_t GetByteIndexed8();
//    void SetByteIndexed8(const uint8_t value);
    uint8_t GetByteIndexed16();
//    void SetByteIndexed16(const uint8_t value);
//    uint8_t GetByteRelative();
//    void SetByteRelative(const uint8_t value);

    // Memory Access
    uint8_t GetByte(const uint16_t addr);
    void SetByte(const uint16_t addr, const uint8_t value);
    uint8_t GetNextByte();

    uint8_t IndirectThreadedCode();
};

#endif // MC68HC705C8_HPP
