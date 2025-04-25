#ifndef CDI_COMMON_VIDEOU32_HPP
#define CDI_COMMON_VIDEOU32_HPP

#include "Video.hpp"

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#if __has_include(<stdbit.h>)
#warning stdbit.h exists, TODO: use it
#endif
#include <vector>

namespace Video
{

// #ifndef __STDC_ENDIAN_NATIVE__
// #error need __STDC_ENDIAN_NATIVE__ compiler support
// #endif

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
//     /*explicit*/ Pixel(uint32_t argb);// : a{argb >> 24}, r{argb >> 16}, g{argb >> 8}, b{argb} {}
//     constexpr Pixel(uint8_t A, uint8_t R, uint8_t G, uint8_t B) : a{A}, r{R}, g{G}, b{B} {}
//
//     uint8_t a, r, g, b;
// #endif // __STDC_ENDIAN_NATIVE__

    constexpr uint32_t IntoU32() const noexcept { return static_cast<uint32_t>(*this); }

    constexpr explicit operator uint32_t() const noexcept { return std::bit_cast<uint32_t>(*this); }

    constexpr Pixel& operator=(uint32_t argb) noexcept
    {
        a = argb >> 24;
        r = argb >> 16;
        g = argb >> 8;
        b = argb;
        return *this;
    }

    constexpr Pixel& operator&=(uint32_t argb) noexcept
    {
        *this = IntoU32() & argb;
        return *this;
    }

    constexpr Pixel& operator|=(uint32_t argb) noexcept
    {
        *this = IntoU32() | argb;
        return *this;
    }
};
static_assert(sizeof(Pixel) == sizeof(uint32_t));

constexpr bool operator==(const Pixel& lhs, uint32_t rhs) noexcept
{
    return static_cast<uint32_t>(lhs) == rhs;
}

constexpr Pixel operator&(const Pixel& lhs, const Pixel& rhs) noexcept
{
    return static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs);
}

constexpr Pixel operator&(const Pixel& lhs, uint32_t rhs) noexcept
{
    return static_cast<uint32_t>(lhs) & rhs;
}

constexpr Pixel operator|(const Pixel& lhs, const Pixel& rhs) noexcept
{
    return static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs);
}

constexpr Pixel operator|(const Pixel& lhs, uint32_t rhs) noexcept
{
    return static_cast<uint32_t>(lhs) | rhs;
}

/** \brief ARGB plane that uses uint32_t to store each pixel. */
class PlaneU32 : public std::vector<Pixel>
{
public:
    static constexpr size_t MAX_WIDTH     = 768;
    static constexpr size_t MAX_HEIGHT    = 560;
    static constexpr size_t CURSOR_WIDTH  = 16;
    static constexpr size_t CURSOR_HEIGHT = 16;

    static constexpr size_t MAX_SIZE  = MAX_WIDTH * MAX_HEIGHT;
    static constexpr size_t CURSOR_SIZE = CURSOR_WIDTH * CURSOR_HEIGHT;

    uint16_t m_width; /**< Width of the plane. */
    uint16_t m_height; /**< Height of the plane. */

    explicit PlaneU32(const uint16_t w = 0, const uint16_t h = 0, const size_t size = MAX_SIZE)
        : std::vector<Pixel>(size, 0), m_width(w), m_height(h)
    {}

    /** \brief Returns a const pointer to the beginning of the given line. */
    const Pixel* GetLinePointer(const size_t line) const noexcept { return data() + line * m_width; }
    /** \brief Returns a pointer to the beginning of the given line. */
    Pixel* GetLinePointer(const size_t line) noexcept { return data() + line * m_width; }

    /** \brief Returns the number of pixels used by the plane. */
    size_t PixelCount() const noexcept { return m_width * m_height; }
};

// Display file decoders.
uint16_t decodeBitmapLineU32(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept;
uint16_t decodeRunLengthLineU32(Pixel* dst, const uint8_t* data, uint16_t width, const uint32_t* CLUTTable, bool is4BPP) noexcept;

uint16_t decodeRGB555LineU32(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width) noexcept;
uint16_t decodeDYUVLineU32(Pixel* dst, const uint8_t* data, uint16_t width, uint16_t initialDYUV) noexcept;
uint16_t decodeCLUTLineU32(Pixel* dst, const uint8_t* data, uint16_t width, const uint32_t* CLUTTable, ImageCodingMethod icm) noexcept;

// Real-Time Decoders (set pixels in ARGB format)
void decodeDYUVU32(Pixel* dst, uint16_t pixel, uint32_t& previous) noexcept;

/** \brief Convert CLUT color to ARGB.
 *
 * \param pixel The CLUT address (must be in the lower bits).
 * \param dst Where the pixel will be written to (alpha channel is 0).
 * \param CLUTTable The CLUT table to use.
 *
 * pixel must have an offset of 128 (or passing &CLUT[128]) when either:
 * - plane B is the plane being drawn.
 * - bit 22 (CLUT select) of register ImageCodingMethod is set for CLUT7+7.
 */
constexpr void decodeCLUTU32(Pixel* dst, const uint8_t pixel, const uint32_t* CLUTTable) noexcept
{
    *dst = CLUTTable[pixel]; // We don't care about transparency for CLUT.
}

void paste(Pixel* dst, const uint16_t dstWidth, const uint16_t dstHeight, const Pixel* src, const uint16_t srcWidth, const uint16_t srcHeight, const uint16_t xOffset = 0, const uint16_t yOffset = 0);

} // namespace Video

#endif // CDI_COMMON_VIDEOU32_HPP
