#ifndef CDI_COMMON_VIDEOSIMD_HPP
#define CDI_COMMON_VIDEOSIMD_HPP

#include "common/utils.hpp"
#include "VideoCommon.hpp"

#include <cstdint>
#include <span>

namespace Video
{

// Display file decoders.
template<uint16_t WIDTH>
uint16_t decodeBitmapLineSIMD(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept;

template<uint16_t WIDTH>
uint16_t decodeRGB555LineSIMD(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB) noexcept;

template<uint16_t WIDTH>
uint16_t decodeDYUVLineLUT(Pixel* dst, const uint8_t* data, uint32_t initialDYUV) noexcept;

} // namespace Video

#endif // CDI_COMMON_VIDEOSIMD_HPP
