#include "Pixel.hpp"

namespace Video
{

/** \brief Compile-time unit test function for Pixel. */
[[maybe_unused]]
static consteval void testPixel()
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

} // namespace Video
