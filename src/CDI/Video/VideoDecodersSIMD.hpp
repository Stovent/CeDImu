#ifndef CDI_COMMON_VIDEOSIMD_HPP
#define CDI_COMMON_VIDEOSIMD_HPP

#include "common/utils.hpp"
#include "VideoCommon.hpp"

#include <cstdint>
#include <span>

namespace Video
{

// Display file decoders.
uint16_t decodeBitmapLineSIMD(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept;

uint16_t decodeRGB555LineSIMD(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width) noexcept;

uint16_t decodeDYUVLineLUT(Pixel* dst, const uint8_t* data, uint16_t width, uint32_t initialDYUV) noexcept;

} // namespace Video

#endif // CDI_COMMON_VIDEOSIMD_HPP
