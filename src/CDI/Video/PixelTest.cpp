#include "Pixel.hpp"

#define ASSERT(cond) if(!(cond)) return false

namespace Video
{

/** \brief Compile-time unit test function for Pixel. */
[[maybe_unused]]
static consteval void testPixelStaticAssert()
{
    static_assert(Pixel{1, 2, 3, 4}.AsU32() == 0x01020304);

    constexpr Pixel p{0x11223344};
    static_assert(p.AsU32() == 0x11223344);
    static_assert(static_cast<uint32_t>(p) == 0x11223344);
    // static_assert(*p.AsU32Pointer() == 0x11223344);
    static_assert(p.a == 0x11);
    static_assert(p.r == 0x22);
    static_assert(p.g == 0x33);
    static_assert(p.b == 0x44);

    static_assert((p & 0xF0F0F0F0) == 0x10203040);
    static_assert((p | 0xF0F0F0F0) == 0xF1F2F3F4);
}

/** \brief Compile-time unit test function for Pixel. */
static consteval bool testPixel()
{
    Pixel argb{0x11, 0x22, 0x33, 0x44};
    Pixel u32{0x11223344};

    ASSERT(u32.AsU32() == 0x11223344);
    ASSERT(argb.AsU32() == 0x11223344);
    ASSERT(u32 == argb);

    ASSERT(argb.a == 0x11);
    ASSERT(argb.r == 0x22);
    ASSERT(argb.g == 0x33);
    ASSERT(argb.b == 0x44);

    ASSERT(u32.a == 0x11);
    ASSERT(u32.r == 0x22);
    ASSERT(u32.g == 0x33);
    ASSERT(u32.b == 0x44);

    argb.a = 0xAA;
    argb.r = 0xBB;
    argb.g = 0xCC;
    argb.b = 0xDD;
    ASSERT(argb.a == 0xAA);
    ASSERT(argb.r == 0xBB);
    ASSERT(argb.g == 0xCC);
    ASSERT(argb.b == 0xDD);
    ASSERT(u32 != argb);

    u32 = 0xAABBCCDD;

    ASSERT(u32.a == 0xAA);
    ASSERT(u32.r == 0xBB);
    ASSERT(u32.g == 0xCC);
    ASSERT(u32.b == 0xDD);
    ASSERT(u32 == argb);

    ASSERT((argb & 0xF0F0F0F0) == 0xA0B0C0D0);
    ASSERT((argb | 0xF0F0F0F0) == 0xFAFBFCFD);
    ASSERT((u32 & 0xF0F0F0F0) == 0xA0B0C0D0);
    ASSERT((u32 | 0xF0F0F0F0) == 0xFAFBFCFD);

    return true;
}
static_assert(testPixel());

} // namespace Video
