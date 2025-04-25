#ifndef CDI_COMMON_VIDEOSIMD_HPP
#define CDI_COMMON_VIDEOSIMD_HPP

#include "Video.hpp"
#include "utils.hpp"
// TODO: move to Video folder.

#include <experimental/simd>
namespace stdx = std::experimental;
#if __cpp_lib_simd
#warning "SIMD is no longer experimental"
#endif

#include <cstdint>

namespace Video
{

// TODO: I know the name is inverted with VideoU32 but np it's a WIP.
using PixelU32 = uint32_t; /**< ARGB 0xAARRGGBB. */
using PixelSIMD = stdx::native_simd<PixelU32>;
inline constexpr size_t SIMD_SIZE = PixelSIMD::size();

/** \brief Increases the given size to be a multiple of SIMD width. */
constexpr size_t SIMDAlign(const size_t size) noexcept
{
    return size + PixelSIMD::size() - (size % PixelSIMD::size());
}

/** \brief SIMD-compatible ARGB plane that uses uint32_t to store each pixel. */
class PlaneSIMD : public std::vector<PixelU32>
{
public:
    static constexpr size_t MAX_WIDTH     = 768;
    static constexpr size_t MAX_HEIGHT    = 560;
    static constexpr size_t CURSOR_WIDTH  = 16;
    static constexpr size_t CURSOR_HEIGHT = 16;

    static constexpr size_t ARGB_MAX_SIZE  = MAX_WIDTH * MAX_HEIGHT;
    static constexpr size_t CURSOR_ARGB_SIZE = CURSOR_WIDTH * CURSOR_HEIGHT;

    uint16_t m_width; /**< Width of the plane. */
    uint16_t m_height; /**< Height of the plane. */

    explicit PlaneSIMD(const uint16_t w = 0, const uint16_t h = 0, const size_t size = ARGB_MAX_SIZE)
        : std::vector<uint32_t>(size, 0), m_width(w), m_height(h)
    {}

    /** \brief Returns a const pointer to the beginning of the given line. */
    const uint32_t* GetLinePointer(const size_t line) const noexcept { return data() + line * m_width; }
    /** \brief Returns a pointer to the beginning of the given line. */
    uint32_t* GetLinePointer(const size_t line) noexcept { return data() + line * m_width; }

    /** \brief Returns the number of pixels used by the plane. */
    size_t PixelCount() const noexcept { return m_width * m_height; }
};

// Display file decoders.
uint16_t decodeBitmapLineSIMD(PixelU32* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept;
uint16_t decodeRunLengthLineSIMD(PixelU32* dst, const uint8_t* data, uint16_t width, const uint32_t* CLUTTable, bool is4BPP) noexcept;

uint16_t decodeRGB555LineSIMD(PixelU32* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width) noexcept;
uint16_t decodeDYUVLineSIMD(PixelU32* dst, const uint8_t* data, uint16_t width, uint16_t initialDYUV) noexcept;
uint16_t decodeCLUTLineSIMD(PixelU32* dst, const uint8_t* data, uint16_t width, const uint32_t* CLUTTable, ImageCodingMethod icm) noexcept;

// Real-Time Decoders (set pixels in ARGB format)
void decodeDYUVSIMD(PixelU32* dst, uint16_t pixel, uint32_t& previous) noexcept;

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
constexpr void decodeCLUTSIMD(PixelU32* dst, const uint8_t pixel, const uint32_t* CLUTTable) noexcept
{
    *dst = CLUTTable[pixel]; // We don't care about transparency for CLUT.
}

void paste(PixelU32* dst, const uint16_t dstWidth, const uint16_t dstHeight, const PixelU32* src, const uint16_t srcWidth, const uint16_t srcHeight, const uint16_t xOffset = 0, const uint16_t yOffset = 0);

} // namespace Video

#endif // CDI_COMMON_VIDEOSIMD_HPP
