#include "VideoDecodersSIMD.hpp"
#include "SIMD.hpp"
#include "VideoDecoders.hpp"
#include "common/utils.hpp"

#include <algorithm>
#include <bit>
#include <execution>
#include <iterator>

#define MAKE_RED_INDEX(Y, V) (as<uint16_t>(Y) << 8 | V)
#define MAKE_GREEN_INDEX(Y, U, V) (as<uint32_t>(Y) << 16 | as<uint32_t>(U) << 8 | V)
#define MAKE_BLUE_INDEX(Y, U) (as<uint16_t>(Y) << 8 | U)

namespace Video
{

static constexpr std::array<uint8_t, 0x1'0000> generateRedLUT() noexcept
{
    std::array<uint8_t, 0x1'0000> array{};
    for(int y = 0; y < 256; ++y)
        for(int v = 0; v < 256; ++v)
            array[MAKE_RED_INDEX(y, v)] = limu8(y + matrixVToR[v]);
    return array;
}

static std::vector<uint8_t> generateGreenLUT() noexcept
{
    std::vector<uint8_t> array{};
    array.resize(256 * 256 * 256);

    for(int y = 0; y < 256; ++y)
        for(int u = 0; u < 256; ++u)
            for(int v = 0; v < 256; ++v)
                array[MAKE_GREEN_INDEX(y, u, v)] = limu8(y - (matrixUToG[u] + matrixVToG[v]));
    return array;
}

static constexpr std::array<uint8_t, 0x1'0000> generateBlueLUT() noexcept
{
    std::array<uint8_t, 0x1'0000> array{};
    for(int y = 0; y < 256; ++y)
        for(int u = 0; u < 256; ++u)
            array[MAKE_BLUE_INDEX(y, u)] = limu8(y + matrixUToB[u]);
    return array;
}

static constexpr std::array<uint8_t, 0x1'0000> redLUT = generateRedLUT();
static std::vector<uint8_t> greenLUT = generateGreenLUT();
static constexpr std::array<uint8_t, 0x1'0000> blueLUT = generateBlueLUT();

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
        return decodeDYUVLineLUT(dst, dataB, width, initialDYUV);

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
    using SIMDFixedU32 = stdx::rebind_simd_t<uint32_t, SIMDNativeU8>;

    const uint8_t* endA = dataA + width;

    const SIMDFixedU32 alphaMask{0x8000};
    for(; dataA < endA; dataA += SIMDNativeU8::size())
    {
        const SIMDNativeU8 da{dataA, stdx::element_aligned};
        const SIMDNativeU8 db{dataB, stdx::element_aligned};

        const SIMDFixedU32 a = stdx::static_simd_cast<uint32_t>(da);
        const SIMDFixedU32 b = stdx::static_simd_cast<uint32_t>(db);
        const SIMDFixedU32 data = a << 8 | b;

        SIMDFixedU32 result = (data & alphaMask);
        result |= data << 9 & 0x00F8'0000;
        result |= data << 6 & 0x0000'F800;
        result |= data << 3 & 0x0000'00F8;

        result.copy_to(dst->AsU32Pointer(), stdx::element_aligned);
    };

    return width;
}

/** \brief Matrixes the YUV values to RGB.
 * The LUT for green is not constexpr because it is a 16MB array which we can't reasonably generate at compile-time.
 */
static constexpr void matrixRGB(Pixel* pixel, const int Y, const uint8_t U, const uint8_t V) noexcept
{
    pixel->r = redLUT[MAKE_RED_INDEX(Y, V)];
    pixel->g = greenLUT[MAKE_GREEN_INDEX(Y, U, V)];
    pixel->b = blueLUT[MAKE_BLUE_INDEX(Y, U)];
}

/** \brief Decode a DYUV line to ARGB using a LUT.
 * \param dst Where the ARGB data will be written to.
 * \param dyuv The source DYUV data.
 * \param width Width of the line in pixels.
 * \param initialDYUV The initial value to be used by the DYUV decoder.
 * \return The number of raw bytes read from \p dyuv.
 *
 * This is not a SIMD decoder as it is impossible because each pixel depends on the decoded value of the previous one.
 * However this is another approach that heavily uses LUTs to remove as much calculations as possible.
 */
uint16_t decodeDYUVLineLUT(Pixel* dst, const uint8_t* data, uint16_t width, uint32_t initialDYUV) noexcept
{
    uint8_t py = bits<16, 23>(initialDYUV);
    uint8_t pu = bits<8, 15>(initialDYUV);
    uint8_t pv = initialDYUV;

    for(uint16_t index = 0; index < width; index += 2)
    {
        const uint8_t high = data[index];
        const uint8_t low = data[index + 1];

        // Green book V.4.4.2
        uint8_t u2 = bits<4, 7>(high);
        uint8_t y1 = bits<0, 3>(high);
        uint8_t v2 = bits<4, 7>(low);
        uint8_t y2 = bits<0, 3>(low);

        y1 = py + dequantizer[y1];
        u2 = pu + dequantizer[u2];
        v2 = pv + dequantizer[v2];
        y2 = y1 + dequantizer[y2];
        const uint8_t u1 = (as<uint16_t>(pu) + as<uint16_t>(u2)) >> 1;
        const uint8_t v1 = (as<uint16_t>(pv) + as<uint16_t>(v2)) >> 1;

        // Store previous.
        py = y2;
        pu = u2;
        pv = v2;

        matrixRGB(dst, y1, u1, v1);
        matrixRGB(&dst[1], y2, u2, v2);
    }

    return width;
}

} // namespace Video
