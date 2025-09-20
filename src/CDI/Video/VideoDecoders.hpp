#ifndef CDI_VIDEO_VIDEODECODERS_HPP
#define CDI_VIDEO_VIDEODECODERS_HPP

#include "VideoCommon.hpp"

#include <array>

namespace Video
{

// Display file decoders.
template<uint16_t WIDTH>
uint16_t decodeBitmapLine(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept;
uint16_t decodeBitmapLine(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept;

template<uint16_t WIDTH, bool RL3>
uint16_t decodeRunLengthLine(Pixel* dst, const uint8_t* data, const uint32_t* CLUTTable) noexcept;
uint16_t decodeRunLengthLine(Pixel* dst, const uint8_t* data, uint16_t width, const uint32_t* CLUTTable, bool is4BPP) noexcept;

template<uint16_t WIDTH>
uint16_t decodeRGB555Line(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB) noexcept;
template<uint16_t WIDTH>
uint16_t decodeDYUVLine(Pixel* dst, const uint8_t* dyuv, uint32_t initialDYUV) noexcept;
template<uint16_t WIDTH>
uint16_t decodeDYUVLineLUT(Pixel* dst, const uint8_t* dyuv, uint32_t initialDYUV) noexcept;
template<uint16_t WIDTH>
uint16_t decodeCLUTLine(Pixel* dst, const uint8_t* data, const uint32_t* CLUTTable, ImageCodingMethod icm) noexcept;

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
constexpr void decodeCLUT(Pixel* dst, const uint8_t pixel, const uint32_t* CLUTTable) noexcept
{
    *dst = CLUTTable[pixel]; // We don't care about transparency for CLUT.
}

void paste(Pixel* dst, const uint16_t dstWidth, const uint16_t dstHeight, const Pixel* src, const uint16_t srcWidth, const uint16_t srcHeight, const uint16_t xOffset = 0, const uint16_t yOffset = 0);

/** \brief Green book Figure V.18 */
inline constexpr std::array<uint8_t, 16> dequantizer{0, 1, 4, 9, 16, 27, 44, 79, 128, 177, 212, 229, 240, 247, 252, 255};

} // namespace Video

#endif // CDI_VIDEO_VIDEODECODERS_HPP
