#ifndef CDI_COMMON_VIDEOSIMD_HPP
#define CDI_COMMON_VIDEOSIMD_HPP

#include "Video.hpp"
#include "utils.hpp"
// TODO: move to Video folder.

#include <cstdint>

namespace Video
{

// Display file decoders.
uint16_t decodeBitmapLineSIMD(uint8_t* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept;
uint16_t decodeRunLengthLineSIMD(uint8_t* dst, const uint8_t* data, uint16_t width, const uint32_t* CLUTTable, bool is4BPP) noexcept;

uint16_t decodeRGB555LineSIMD(uint8_t* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width) noexcept;
uint16_t decodeDYUVLineSIMD(uint8_t* dst, const uint8_t* data, uint16_t width, uint16_t initialDYUV) noexcept;
uint16_t decodeCLUTLineSIMD(uint8_t* dst, const uint8_t* data, uint16_t width, const uint32_t* CLUTTable, ImageCodingMethod icm) noexcept;

// Real-Time Decoders (set pixels in ARGB format)
void decodeDYUVSIMD(const uint16_t pixel, uint8_t pixels[8], uint32_t& previous) noexcept;

/** \brief Convert CLUT color to ARGB.
 *
 * \param pixel The CLUT address (must be in the lower bits).
 * \param pixels Where the pixel will be written to. pixels[0] = 0xFF, pixels[1] = red, pixels[2] = green, pixels[3] = blue.
 * \param CLUTTable The CLUT table to use.
 *
 * pixel must have an offset of 128 (or passing &CLUT[128]) when either:
 * - plane B is the plane being drawn.
 * - bit 22 (CLUT select) of register ImageCodingMethod is set for CLUT7+7.
 */
constexpr void decodeCLUTSIMD(const uint8_t pixel, uint8_t pixels[4], const uint32_t* CLUTTable) noexcept
{
    pixels[0] = 0xFF;
    pixels[1] = bits<16, 23>(CLUTTable[pixel]);
    pixels[2] = bits<8, 15>(CLUTTable[pixel]);
    pixels[3] = CLUTTable[pixel];
}

} // namespace Video

#endif // CDI_COMMON_VIDEOSIMD_HPP
