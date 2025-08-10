/** \file utilsTest.cpp
 * \brief Unit tests for the utils functions.
 */

#include "utils.hpp"

#include <array>

#define ASSERT(cond) if(!(cond)) return false

static consteval bool testSignExtend()
{
    ASSERT((signExtend<int8_t, uint32_t>(int8_t(0x7F)) == 0x0000'007F));
    ASSERT((signExtend<int8_t, uint32_t>(int8_t(0x80)) == 0xFFFF'FF80));
    ASSERT((signExtend<int16_t, uint32_t>(int16_t(0x7FFF)) == 0x0000'7FFF));
    ASSERT((signExtend<int16_t, uint32_t>(int16_t(0x8000)) == 0xFFFF'8000));
    return true;
}
static_assert(testSignExtend());

static consteval bool testZeroExtend()
{
    ASSERT((zeroExtend<uint8_t, uint32_t>(uint8_t(0x7F)) == 0x0000'007F));
    ASSERT((zeroExtend<uint8_t, uint32_t>(uint8_t(0x80)) == 0x0000'0080));
    ASSERT((zeroExtend<uint16_t, uint32_t>(uint16_t(0x7FFF)) == 0x0000'7FFF));
    ASSERT((zeroExtend<uint16_t, uint32_t>(uint16_t(0x8000)) == 0x0000'8000));
    return true;
}
static_assert(testZeroExtend());

static consteval bool testBit()
{
    ASSERT(!bit<0>(0b10));
    ASSERT( bit<0>(0b01));
    ASSERT(!bit<31>(0x7FFF'FFFF));
    ASSERT( bit<31>(0x8000'0000));
    return true;
}
static_assert(testBit());

static consteval bool testBits()
{
    ASSERT((bits<1, 2>(0b1001) == 0));
    ASSERT((bits<1, 2>(0b0110) == 3));
    ASSERT((bits<29, 30>(0x9FFF'FFFF) == 0));
    ASSERT((bits<29, 30>(0x6000'0000) == 3));
    return true;
}
static_assert(testBits());

static consteval bool testPBCDToByte()
{
    ASSERT(PBCDToByte(0x01) == 1);
    ASSERT(PBCDToByte(0x09) == 9);
    ASSERT(PBCDToByte(0x0A) == 10);
    ASSERT(PBCDToByte(0x10) == 10);
    ASSERT(PBCDToByte(0x90) == 90);
    ASSERT(PBCDToByte(0x99) == 99);
    ASSERT(PBCDToByte(0xAA) == 110);
    return true;
}
static_assert(testPBCDToByte());

static consteval bool testByteToPBCD()
{
    ASSERT(byteToPBCD(1) == 0x1);
    ASSERT(byteToPBCD(9) == 0x9);
    ASSERT(byteToPBCD(10) == 0x10);
    ASSERT(byteToPBCD(11) == 0x11);
    ASSERT(byteToPBCD(99) == 0x99);
    ASSERT(byteToPBCD(100) == 0x00);
    return true;
}
static_assert(testByteToPBCD());

static consteval bool testIsEven()
{
    ASSERT( isEven(0));
    ASSERT(!isEven(1));
    ASSERT( isEven(-2));
    ASSERT(!isEven(-1));
    return true;
}
static_assert(testIsEven());

static consteval bool testLimu8()
{
    ASSERT(limu8(INT_MIN) == 0);
    ASSERT(limu8(-1) == 0);
    ASSERT(limu8(0) == 0);
    ASSERT(limu8(128) == 128);
    ASSERT(limu8(UINT8_MAX) == UINT8_MAX);
    ASSERT(limu8(256) == UINT8_MAX);
    ASSERT(limu8(INT_MAX) == UINT8_MAX);
    return true;
}
static_assert(testLimu8());

static consteval bool testLims16()
{
    ASSERT(lims16(INT_MIN) == INT16_MIN);
    ASSERT(lims16(INT16_MIN - 1) == INT16_MIN);
    ASSERT(lims16(INT16_MIN) == INT16_MIN);
    ASSERT(lims16(-1) == -1);
    ASSERT(lims16(0) == 0);
    ASSERT(lims16(1) == 1);
    ASSERT(lims16(INT16_MAX) == INT16_MAX);
    ASSERT(lims16(INT16_MAX + 1) == INT16_MAX);
    ASSERT(lims16(INT_MAX) == INT16_MAX);
    return true;
}
static_assert(testLims16());

static consteval bool testMakeU16()
{
    ASSERT(makeU16(0x10, 0x32) == 0x3210);
    return true;
}
static_assert(testMakeU16());

static consteval bool testMakeU32()
{
    ASSERT(makeU32(0x10, 0x32, 0x54, 0x76) == 0x76543210);
    return true;
}
static_assert(testMakeU32());

static constexpr std::array<uint8_t, 8> ARRAY{8, 7, 6, 5, 4, 3, 2, 1};

static consteval bool testGetArray16()
{
    ASSERT(getArray16(ARRAY, 2) == 0x0605);
    return true;
}
static_assert(testGetArray16());

static consteval bool testGetArray32()
{
    ASSERT(getArray32(ARRAY, 2) == 0x06050403);
    return true;
}
static_assert(testGetArray32());
