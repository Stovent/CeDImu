/** \file Kernel.cpp
 * \brief Unit tests of the Kernel.hpp header.
 */

#include "Kernel.hpp"

#include <algorithm>
#include <array>
#include <cstdint>

#define ASSERT(cond) if(!(cond)) return false

static constexpr std::array<uint8_t, 0x30> MODULE_HEADER{
    0x4A, 0xFC, 0x00, 0x01, 0x00, 0x00, 0x11, 0x9E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x94,
    0x05, 0x55, 0x0D, 0x01, 0xA0, 0x03, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1D, 0x3F,
};

template<typename T = uint8_t>
struct EmulatedMemory : public OS9::EmulatedMemoryAccess
{
    std::array<T, 256> memory{};

    constexpr EmulatedMemory()
    {
        GetByte = [this] (uint32_t addr) { return memory.at(addr); };
        GetWord = [this] (uint32_t addr) { return GET_ARRAY16(memory, addr); };
        SetByte = [this] (uint32_t addr, uint8_t data) { memory.at(addr) = data; };
        SetWord = [this] (uint32_t addr, uint16_t data) { memory.at(addr) = data >> 8; memory.at(addr + 1) = data; };
    }
};

static consteval bool testU32()
{
    EmulatedMemory memory{};

    OS9::U32 data{memory, 0x30};
    ASSERT(data.Address() == 0x30);
    ASSERT(memory.memory[0x30] == 0);
    ASSERT(memory.memory[0x31] == 0);
    ASSERT(memory.memory[0x32] == 0);
    ASSERT(memory.memory[0x33] == 0);
    ASSERT(data.Read() == 0);
    ASSERT(data == 0);

    data = 0x12345678;
    ASSERT(memory.memory[0x30] == 0x12);
    ASSERT(memory.memory[0x31] == 0x34);
    ASSERT(memory.memory[0x32] == 0x56);
    ASSERT(memory.memory[0x33] == 0x78);
    ASSERT(data.Read() == 0x12345678);
    ASSERT(data == 0x12345678);

    return true;
}
static_assert(testU32());

static consteval bool testPointer()
{
    EmulatedMemory memory{};

    OS9::Pointer<OS9::U32> ptr{memory, 0x34};
    ASSERT(ptr.PointerAddress() == 0x34);
    ASSERT(ptr.TargetAddress() == 0);
    ptr = 0x30;
    ASSERT(ptr.PointerAddress() == 0x34);
    ASSERT(ptr.TargetAddress() == 0x30);
    *ptr = 0x12345678;
    ASSERT(*ptr == 0x12345678);

    *ptr = 84;
    ASSERT((*ptr).Read() == 84);

    OS9::Pointer<OS9::U32> ptrEnd{memory, 0x38};
    ASSERT(ptrEnd.TargetAddress() == 0);
    ptrEnd = 0x40;
    ASSERT(ptrEnd.TargetAddress() == 0x40);
    ASSERT((ptrEnd - ptr) == 4);
    ptrEnd = 0x30;
    ASSERT(ptrEnd == ptr);
    // ++ptrEnd;
    // ASSERT(ptrEnd.TargetAddress() == 0x34);

    return true;
}
static_assert(testPointer());

static consteval bool testString()
{
    static constexpr char ALPHABET[] = "abcdefghijklmnopqrstuvwxyz";
    EmulatedMemory<char> memory{};
    std::copy_n(ALPHABET, sizeof ALPHABET, memory.memory.data());

    // After the string
    OS9::Pointer<OS9::CString> ptr{memory, 0x30};
    // ptr is 0.
    ASSERT(memory.memory[0] == 'a');
    ASSERT(memory.memory[1] == 'b');
    ASSERT(memory.memory[25] == 'z');
    ASSERT(memory.memory[26] == 0);
    ASSERT((*ptr).Read() == ALPHABET);
    ASSERT(*ptr == ALPHABET);

    return true;
}
static_assert(testString());

static consteval bool testModule()
{
    EmulatedMemory memory{};
    std::copy(MODULE_HEADER.begin(), MODULE_HEADER.end(), memory.memory.begin());

    // After the dummy module header.
    OS9::Pointer<OS9::Module> ptr{memory, 0x30};
    ASSERT(ptr.TargetAddress() == 0);

    OS9::Module module = *ptr;
    ASSERT(module.M_ID == 0x4AFC);

    return true;
}
static_assert(testModule());
