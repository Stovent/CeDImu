#ifndef CDI_VIDEO_PIXEL_HPP
#define CDI_VIDEO_PIXEL_HPP

#include <bit>
#include <cstdint>
#if __has_include(<stdbit.h>)
#warning stdbit.h exists, TODO: use it
#endif

// #ifndef __STDC_ENDIAN_NATIVE__
// #error need __STDC_ENDIAN_NATIVE__ compiler support
// #endif

// #if __STDC_ENDIAN_NATIVE__ != __STDC_ENDIAN_LITTLE__ && __STDC_ENDIAN_NATIVE__ != __STDC_ENDIAN_BIG__
// #error requires a standard little-endian or big-endian machine
// #endif

namespace Video
{

/** \brief Represents an ARGB (0xAARRGGBB) pixel.
 * TODO: this is only supported on little-endian machines until __STDC_ENDIAN_NATIVE__ is available.
 */
struct alignas(uint32_t) Pixel
{
    constexpr Pixel() : Pixel(0) {}

// #if __STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_LITTLE__
    constexpr Pixel(uint32_t argb) : b(argb), g(argb >> 8), r(argb >> 16), a(argb >> 24) {}
    constexpr Pixel(uint8_t A, uint8_t R, uint8_t G, uint8_t B) : b{B}, g{G}, r{R}, a{A} {}

    uint8_t b, g, r, a;
// #else
//     constexpr Pixel(uint32_t argb) : a(argb >> 24), r(argb >> 16), g(argb >> 8), b(argb) {}
//     constexpr Pixel(uint8_t A, uint8_t R, uint8_t G, uint8_t B) : a{A}, r{R}, g{G}, b{B} {}
//
//     uint8_t a, r, g, b;
// #endif // __STDC_ENDIAN_NATIVE__

    constexpr uint32_t AsU32() const noexcept { return static_cast<uint32_t>(*this); }

    constexpr explicit operator uint32_t() const noexcept { return std::bit_cast<uint32_t>(*this); }

    constexpr Pixel& operator=(const uint32_t argb) noexcept
    {
        a = argb >> 24;
        r = argb >> 16;
        g = argb >> 8;
        b = argb;
        return *this;
    }

    constexpr Pixel& operator&=(const uint32_t argb) noexcept
    {
        *this = AsU32() & argb;
        return *this;
    }

    constexpr Pixel& operator|=(const uint32_t argb) noexcept
    {
        *this = AsU32() | argb;
        return *this;
    }
};
static_assert(sizeof(Pixel) == sizeof(uint32_t));

constexpr bool operator==(const Pixel& lhs, const uint32_t rhs) noexcept
{
    return static_cast<uint32_t>(lhs) == rhs;
}

constexpr Pixel operator&(const Pixel& lhs, const Pixel& rhs) noexcept
{
    return static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs);
}

constexpr Pixel operator&(const Pixel& lhs, const uint32_t rhs) noexcept
{
    return static_cast<uint32_t>(lhs) & rhs;
}

constexpr Pixel operator|(const Pixel& lhs, const Pixel& rhs) noexcept
{
    return static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs);
}

constexpr Pixel operator|(const Pixel& lhs, const uint32_t rhs) noexcept
{
    return static_cast<uint32_t>(lhs) | rhs;
}

} // namespace Video

#endif // CDI_VIDEO_PIXEL_HPP
