/** \file Kernel.cpp
 * \brief Unit tests of the Kernel.hpp header.
 */

#include "Kernel.hpp"

#include <array>
#include <cstdint>

#define ASSERT(cond) if(!(cond)) return false

static constexpr std::array<uint8_t, 0x30> MODULE_HEADER{
    0x4A, 0xFC, 0x00, 0x01, 0x00, 0x00, 0x11, 0x9E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x94,
    0x05, 0x55, 0x0D, 0x01, 0xA0, 0x03, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1D, 0x3F,
};

struct EmulatedMemory : public OS9::MemoryAccess
{
    std::array<uint8_t, 256> memory{};

    virtual constexpr uint8_t GetByte(uint32_t addr) const noexcept override
    {
        return memory[addr];
    }

    virtual constexpr uint16_t GetWord(uint32_t addr) const noexcept override
    {
        return GET_ARRAY16(memory, addr);
    }


    virtual constexpr void SetByte(uint32_t addr, uint8_t data) noexcept override
    {
        memory.at(addr) = data;
    }

    virtual constexpr void SetWord(uint32_t addr, uint16_t data) noexcept override
    {
        memory.at(addr) = data >> 8;
        memory.at(addr + 1) = data;
    }
};

static constexpr bool test()
{
    EmulatedMemory memory{};

    // After the dummy module.
    OS9::U8 byte{memory, 0x30};
    ASSERT(memory.memory[0x30] == 0);
    ASSERT(byte.Read() == 0);
    ASSERT(byte == 0);

    byte = 42;
    ASSERT(memory.memory[0x30] == 42);
    ASSERT(byte.Read() == 42);
    ASSERT(byte == 42);

    OS9::Pointer<OS9::U8> ptr{memory, 0x34};
    ASSERT(ptr.TargetAddress() == 0);
    ptr = 0x30;
    ASSERT(ptr.TargetAddress() == 0x30);
    ASSERT((*ptr) == 42);

    *ptr = 84;
    ASSERT((*ptr).Read() == 84);

    return true;
}

static constexpr bool testModule()
{
    EmulatedMemory memory{};
    std::copy(MODULE_HEADER.begin(), MODULE_HEADER.end(), memory.memory.begin());

    OS9::Pointer<OS9::Module> ptr{memory, 0x30};
    ASSERT(ptr.TargetAddress() == 0);

    OS9::Module module = *ptr;
    ASSERT(module.M_ID == 0x4AFC);

    return true;
}

static_assert(test());
static_assert(testModule());
