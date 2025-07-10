#include "VideoDecodersSIMD.hpp"
#include "SIMD.hpp"
#include "VideoDecoders.hpp"
#include "common/utils.hpp"

#include <algorithm>
#include <bit>
#include <execution>
#include <iterator>

namespace Video
{

/** \brief Decode a bitmap file line.
 *
 * \param dst Where the decoded line will be written to in ARGB.
 * \param dataA See below.
 * \param dataB See below.
 * \param width The width of the line.
 * \param CLUTTable The CLUT table to use.
 * \param initialDYUV The initial value to be used by the DYUV decoder.
 * \param icm The coding method of the file.
 * \return The number of raw bytes read from dataB (and dataA in RGB555).
 *
 * If \p icm is ImageCodingMethod::CLUT77 or the video plane is plane B, then send `&CLUT[128]` as the \p CLUTTable.
 *
 * If the coding method is RGB555, dataA must contain the channel A data and dataB must contain the channel B data.
 * If it is not RGB555, dataB is the source data and dataA remain unused.
 */
uint16_t decodeBitmapLineSIMD(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept
{
    if(icm == ImageCodingMethod::DYUV)
        return decodeDYUVLine(dst, dataB, width, initialDYUV);

    if(icm == ImageCodingMethod::RGB555)
        return decodeRGB555LineSIMD(dst, dataA, dataB, width);

    return decodeCLUTLine(dst, dataB, width, CLUTTable, icm);
}

/** \brief Decode a RGB555 line to ARGB using SIMD.
 * \param dst Where the ARGB data will be written to.
 * \param dataA The plane A data (high order byte of the pixel).
 * \param dataB The plane B data (low order byte of the pixel).
 * \param width Width of the line in pixels.
 * \return The number of raw bytes read from each data source.
 * \attention \p dst, \p dataA and \p dataB are written/read in chunks of std::simd::size(). Make sure the buffers can be read and written beyond the actual line length.
 * The transparency bit is set in the alpha byte (0x80 when the bit is 1, 0 otherwise).
 */
uint16_t decodeRGB555LineSIMD(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width) noexcept
{
    // TODO: ensure we do not index out of bound.
    using SIMDFixedU32 = stdx::fixed_size_simd<uint32_t, SIMDNativeU8::size()>;

    const uint8_t* endA = dataA + width;

    static const SIMDNativeU8 alphaMask{0x80};
    for(; dataA < endA; dataA += SIMDNativeU8::size())
    {
        SIMDNativeU8 a{dataA, stdx::element_aligned};
        SIMDNativeU8 b{dataB, stdx::element_aligned};

        // TODO: should I use simd_mask anyway?
        SIMDNativeU8 alpha = (a & alphaMask);
        SIMDNativeU8 red = a << 1 & 0xF8;
        SIMDNativeU8 green = (a << 6) | (b >> 5 & 0x38);
        SIMDNativeU8 blue = b << 3;
        SIMDFixedU32 result = stdx::static_simd_cast<uint32_t>(alpha) << 24 |
                              stdx::static_simd_cast<uint32_t>(red) << 16 |
                              stdx::static_simd_cast<uint32_t>(green) << 8 |
                              stdx::static_simd_cast<uint32_t>(blue);

        result.copy_to(dst->AsU32Pointer(), stdx::element_aligned);
    };

    return width;
}

} // namespace Video
