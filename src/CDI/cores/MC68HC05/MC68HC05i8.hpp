#ifndef CDI_CORES_MC68HC05_MC68HC05I8_HPP
#define CDI_CORES_MC68HC05_MC68HC05I8_HPP

class CDI;
#include "MC68HC05.hpp"

#include <array>

class MC68HC05i8 : protected MC68HC05
{
public:
    MC68HC05i8() = delete;
    MC68HC05i8(const MC68HC05i8&) = delete;
    MC68HC05i8(const void* bios, uint16_t size);

    void Reset();

    void IncrementTime(double ns);

    virtual uint8_t GetMemory(uint16_t addr) override;
    virtual void SetMemory(uint16_t addr, uint8_t value) override;

private:
    std::array<uint8_t, 0x4000> memory;
    size_t pendingCycles;
};

#endif // CDI_CORES_MC68HC05_MC68HC05I8_HPP
