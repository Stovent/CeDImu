#ifndef CDI_VIDEO_PIXEL_HPP
#define CDI_VIDEO_PIXEL_HPP

#include <bit>
#include <cstdint>
#include <stdbit.h>

#if __STDC_VERSION_STDBIT_H__ < 202311L
#error need __STDC_ENDIAN_NATIVE__ compiler support
#endif

#if __STDC_ENDIAN_NATIVE__ != __STDC_ENDIAN_LITTLE__ && __STDC_ENDIAN_NATIVE__ != __STDC_ENDIAN_BIG__
#error requires a regular little-endian or big-endian machine
#endif

namespace Video
{

/** \brief Represents an ARGB (0xAARRGGBB) pixel.
 */
struct alignas(uint32_t) Pixel
{
    /** \brief Type alias to allow for raw manipulation. */
    using ARGB32 = uint32_t;

    constexpr Pixel() : Pixel(0) {}

#if __STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_LITTLE__
    constexpr Pixel(ARGB32 argb) : b(argb), g(argb >> 8), r(argb >> 16), a(argb >> 24) {}
    constexpr Pixel(uint8_t A, uint8_t R, uint8_t G, uint8_t B) : b{B}, g{G}, r{R}, a{A} {}

    uint8_t b, g, r, a;
#else
    constexpr Pixel(ARGB32 argb) : a(argb >> 24), r(argb >> 16), g(argb >> 8), b(argb) {}
    constexpr Pixel(uint8_t A, uint8_t R, uint8_t G, uint8_t B) : a{A}, r{R}, g{G}, b{B} {}

    uint8_t a, r, g, b;
#endif // __STDC_ENDIAN_NATIVE__

    constexpr ARGB32 AsU32() const noexcept { return static_cast<ARGB32>(*this); }

    constexpr explicit operator ARGB32() const noexcept { return std::bit_cast<ARGB32>(*this); }

    // reinterpret_cast is not constexpr compatible.
    const ARGB32* AsU32Pointer() const noexcept { return reinterpret_cast<const ARGB32*>(this); }
    ARGB32* AsU32Pointer() noexcept { return reinterpret_cast<ARGB32*>(this); }

    constexpr bool operator==(const Pixel& other) const = default;

    constexpr Pixel& operator=(const ARGB32 argb) noexcept
    {
        if consteval
        {
            a = argb >> 24;
            r = argb >> 16;
            g = argb >> 8;
            b = argb;
        }
        else
        {
            *AsU32Pointer() = argb;
        }

        return *this;
    }

    constexpr Pixel& operator&=(const ARGB32 argb) noexcept
    {
        *this = AsU32() & argb;
        return *this;
    }

    constexpr Pixel& operator|=(const ARGB32 argb) noexcept
    {
        *this = AsU32() | argb;
        return *this;
    }
};
static_assert(sizeof(Pixel) == sizeof(Pixel::ARGB32));

constexpr bool operator==(const Pixel& lhs, const Pixel::ARGB32 rhs) noexcept
{
    return static_cast<Pixel::ARGB32>(lhs) == rhs;
}

constexpr Pixel operator&(const Pixel& lhs, const Pixel& rhs) noexcept
{
    return static_cast<Pixel::ARGB32>(lhs) & static_cast<Pixel::ARGB32>(rhs);
}

constexpr Pixel operator&(const Pixel& lhs, const Pixel::ARGB32 rhs) noexcept
{
    return static_cast<Pixel::ARGB32>(lhs) & rhs;
}

constexpr Pixel operator|(const Pixel& lhs, const Pixel& rhs) noexcept
{
    return static_cast<Pixel::ARGB32>(lhs) | static_cast<Pixel::ARGB32>(rhs);
}

constexpr Pixel operator|(const Pixel& lhs, const Pixel::ARGB32 rhs) noexcept
{
    return static_cast<Pixel::ARGB32>(lhs) | rhs;
}

} // namespace Video

#endif // CDI_VIDEO_PIXEL_HPP
