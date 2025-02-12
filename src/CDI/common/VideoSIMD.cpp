#include "VideoSIMD.hpp"
#include "utils.hpp"

#include <algorithm>
#include <bit>
#include <execution>
#include <iterator>

#include <experimental/simd>
namespace stdx = std::experimental;
#if __cpp_lib_simd
#warning "SIMD is no longer experimental"
#endif

/** \brief An iterator that counts numbers. */
template<typename NUM, NUM INC = 1>
class CountingIterator
{
public:
    static_assert(INC != 0, "Increment must not be 0");

    using value_type = NUM;

    constexpr CountingIterator() : m_count{0} {}
    constexpr explicit CountingIterator(const value_type& min) : m_count{min} {}

    constexpr const value_type& operator*() const noexcept { return m_count; }

    constexpr CountingIterator& operator++() noexcept { m_count += INC; return *this; }
    constexpr CountingIterator& operator++(int) noexcept { m_count += INC; return *this; }

    constexpr bool operator==(const CountingIterator& other) noexcept { return m_count == other.m_count; }
    constexpr bool operator!=(const CountingIterator& other) noexcept { return m_count != other.m_count; }

private:
    value_type m_count;
};

namespace Video
{

/** \brief Green book Figure V.18 */
static constexpr std::array<uint8_t, 16> dequantizer{0, 1, 4, 9, 16, 27, 44, 79, 128, 177, 212, 229, 240, 247, 252, 255};

static constexpr std::array<int, 256> generateVToR() noexcept
{
    std::array<int, 256> array;
    for(int i = 0; i < 256; i++)
        array[i] = (351 * (i - 128)) / 256;
    return array;
}

static constexpr std::array<int, 256> generateVToG() noexcept
{
    std::array<int, 256> array;
    for(int i = 0; i < 256; i++)
        array[i] = (179 * (i - 128)) / 256;
    return array;
}

static constexpr std::array<int, 256> generateUToG() noexcept
{
    std::array<int, 256> array;
    for(int i = 0; i < 256; i++)
        array[i] = (86 * (i - 128)) / 256;
    return array;
}

static constexpr std::array<int, 256> generateUToB() noexcept
{
    std::array<int, 256> array;
    for(int i = 0; i < 256; i++)
        array[i] = (444 * (i - 128)) / 256;
    return array;
}

static constexpr std::array<int, 256> matrixVToR = generateVToR();
static constexpr std::array<int, 256> matrixVToG = generateVToG();
static constexpr std::array<int, 256> matrixUToG = generateUToG();
static constexpr std::array<int, 256> matrixUToB = generateUToB();

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
uint16_t decodeBitmapLineSIMD(uint8_t* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept
{
    if(icm == ImageCodingMethod::DYUV)
        return decodeDYUVLineSIMD(dst, dataB, width, initialDYUV);

    if(icm == ImageCodingMethod::RGB555)
        return decodeRGB555LineSIMD(dst, dataA, dataB, width);

    return decodeCLUTLineSIMD(dst, dataB, width, CLUTTable, icm);
}

/** \brief Decode a Run-length file line.
 *
 * \param dst Where the decoded line will be written in ARGB.
 * \param data The raw input data to be decoded.
 * \param width The width of the line.
 * \param CLUTTable The CLUT table to use.
 * \param is4BPP true for RL3, false for RL7.
 * \return The number of raw bytes read from data.
 */
uint16_t decodeRunLengthLineSIMD(uint8_t* dst, const uint8_t* data, uint16_t width, const uint32_t* CLUTTable, bool is4BPP) noexcept
{
    uint16_t index = 0;

    if(is4BPP) // RL3
    {
        for(int x = 0; x < width;)
        {
            const uint8_t format = data[index++];
            const uint8_t color1 = bits<4, 6>(format);
            const uint8_t color2 = bits<0, 2>(format);
            uint16_t count = 0; // number of memcpy to do.

            if(bit<7>(format)) // run of pixels pairs
            {
                count = data[index++];
                if(count == 0)
                    count = width - x;
                --count; // one memcpy less than the pixels because of the initial decode.
            }

            uint8_t* pixels = &dst[x++ * 4];
            uint8_t* pixels2 = &dst[x++ * 4];
            decodeCLUT(color1, pixels, CLUTTable);
            decodeCLUT(color2, pixels2, CLUTTable);
            for(int i = 0; i < count; i++)
            {
                memcpy(&dst[x * 4], pixels, 8);
                x += 2;
            }
        }
    }
    else // RL7
    {
        for(int x = 0; x < width;)
        {
            const uint8_t format = data[index++];
            const uint8_t color = bits<0, 6>(format);
            uint16_t count = 0; // number of memcpy to do.

            if(bit<7>(format)) // run of single pixels
            {
                count = data[index++];
                if(count == 0)
                    count = width - x;
                --count; // one memcpy less than the pixels because of the initial decode.
            }

            uint8_t* pixels = &dst[x++ * 4];
            decodeCLUT(color, pixels, CLUTTable);
            for(int i = 0; i < count; i++)
            {
                memcpy(&dst[x++ * 4], pixels, 4);
            }
        }
    }

    return index;
}

/** \brief Decode a RGB555 line to ARGB using SIMD.
 * \param dst Where the ARGB data will be written to.
 * \param dataA The plane A data (high order byte of the pixel).
 * \param dataB The plane B data (low order byte of the pixel).
 * \param width Width of the line in pixels.
 * \return The number of raw bytes read from each data source.
 * \attention \p dst, \p dataA and \p dataB are written/read in chunks of std::simd::size(). Make sure the buffers can be read and written beyond the actual line length.
 */
uint16_t decodeRGB555LineSIMD(uint8_t* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width) noexcept
{
    // TODO: ensure we do not index out of bound.
    using SIMD8 = stdx::native_simd<uint8_t>;
    using FIXED32 = stdx::fixed_size_simd<uint32_t, SIMD8::size()>;
    const uint8_t* endA = dataA + width;

    static_assert(std::endian::native == std::endian::little);

    static const SIMD8 alphaMask{0x80};
    for(; dataA < endA; dataA += SIMD8::size())
    {
        SIMD8 a{dataA, stdx::element_aligned};
        SIMD8 b{dataB, stdx::element_aligned};

        // TODO: should I use simd_mask anyway?
        SIMD8 alpha = (a & alphaMask);
        SIMD8 red = a << 1 & 0xF8;
        SIMD8 green = (a << 6) | (b >> 5 & 0x38);
        SIMD8 blue = b << 3;
        FIXED32 result = stdx::simd_cast<uint32_t>(blue) << 24 |
                         stdx::simd_cast<uint32_t>(green) << 16 |
                         stdx::simd_cast<uint32_t>(red) << 8 |
                         stdx::simd_cast<uint32_t>(alpha); // Little endian.

        result.copy_to(reinterpret_cast<uint32_t*>(dst), stdx::overaligned<sizeof(uint8_t)>);
    };

    return width;
}

/** \brief Decode a DYUV line to ARGB.
 * \param dst Where the ARGB data will be written to.
 * \param dyuv The source DYUV data.
 * \param width Width of the line in pixels.
 * \param initialDYUV The initial value to be used by the DYUV decoder.
 * \return The number of raw bytes read from \p dyuv.
 */
uint16_t decodeDYUVLineSIMD(uint8_t* dst, const uint8_t* dyuv, uint16_t width, uint16_t initialDYUV) noexcept
{
    uint32_t previous = initialDYUV;

    CountingIterator<int, 2> it{0};

    std::for_each_n(std::execution::seq, it, width >> 1, [dst, dyuv, &previous] (int index) {
        uint16_t pixel = as<uint16_t>(dyuv[index]) << 8;
        pixel |= dyuv[index + 1];
        decodeDYUVSIMD(pixel, &dst[index * 4], previous);
    });

    return width;
}

/** \brief Decode a CLUT line to ARGB.
 * \param dst Where the ARGB data will be written to.
 * \param data The source CLUT data.
 * \param width Width of the line in pixels.
 * \param CLUTTable The CLUT to use when decoding.
 * \param icm The coding method (non-CLUT values are ignored and treated as CLUT7).
 * \return The number of raw bytes read from data.
 *
 * If \p icm is ImageCodingMethod::CLUT77 or the video plane is plane B, then sent `&CLUT[128]` as the \p CLUTTable.
 */
uint16_t decodeCLUTLineSIMD(uint8_t* dst, const uint8_t* data, uint16_t width, const uint32_t* CLUTTable, ImageCodingMethod icm) noexcept
{
    if(icm == ImageCodingMethod::CLUT4)
    {
        CountingIterator<int, 2> it{0};
        std::for_each_n(std::execution::unseq, it, width >> 1, [dst, data, CLUTTable] (int index) {
            const uint8_t d = data[index];
            const uint8_t color1 = bits<4, 7>(d);
            const uint8_t color2 = bits<0, 3>(d);
            index *= 4;
            decodeCLUTSIMD(color1, &dst[index], CLUTTable);
            decodeCLUTSIMD(color2, &dst[index + 4], CLUTTable);
        });

        return width / 2;
    }
    else
    {
        const uint8_t colorMask = icm == ImageCodingMethod::CLUT8 ? 0xFF : 0x7F;

        CountingIterator<int> it{0};
        std::for_each_n(std::execution::unseq, it, width, [dst, data, CLUTTable, colorMask] (const int index) {
            const uint8_t color = data[index] & colorMask;
            decodeCLUTSIMD(color, &dst[index * 4], CLUTTable);
        });

        return width;
    }
}

static constexpr void matrixRGBSIMD(const int Y, const uint8_t U, const uint8_t V, uint8_t pixels[4]) noexcept
{
    pixels[0] = 0xFF;
    pixels[1] = limu8(Y + matrixVToR[V]);
    pixels[2] = limu8(Y - (matrixUToG[U] + matrixVToG[V]));
    pixels[3] = limu8(Y + matrixUToB[U]);
}

/** \brief Convert DYUV to ARGB.
 *
 * \param pixel The pixel to decode.
 * \param pixels Where the pixels will be written to. pixels[0,4] = 0xFF, pixels[1,5] = red, pixels[2,6] = green, pixels[3,7] = blue.
 * \param previous The previous pixel colors.
 */
void decodeDYUVSIMD(const uint16_t pixel, uint8_t pixels[8], uint32_t& previous) noexcept
{
    // Green book V.4.4.2
    uint8_t u2 = bits<12, 15>(pixel);
    uint8_t y1 = bits<8, 11>(pixel);
    uint8_t v2 = bits<4, 7>(pixel);
    uint8_t y2 = bits<0, 3>(pixel);

    const uint8_t py = bits<16, 23>(previous);
    const uint8_t pu = bits<8, 15>(previous);
    const uint8_t pv = previous;

    y1 = py + dequantizer[y1];
    u2 = pu + dequantizer[u2];
    v2 = pv + dequantizer[v2];
    y2 = y1 + dequantizer[y2];
    const uint8_t u1 = (as<uint16_t>(pu) + as<uint16_t>(u2)) >> 1;
    const uint8_t v1 = (as<uint16_t>(pv) + as<uint16_t>(v2)) >> 1;
    previous = as<uint32_t>(y2) << 16 | as<uint32_t>(u2) << 8 | v2;

    matrixRGBSIMD(y1, u1, v1, pixels);
    matrixRGBSIMD(y2, u2, v2, &pixels[4]);
}

} // namespace Video
