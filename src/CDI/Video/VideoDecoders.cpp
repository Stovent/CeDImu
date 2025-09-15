/** \file VideoDecoders.cpp
 * \brief Implementation of video-decoding functions.
 *
 * \warning For bitmap and runlength functions, the destination pixels are always written in double resolution (720 or
 * 768 pixels), no matter the source width.
 * When the source width is normal resolution, the pixels are simply duplicated.
 *
 * TODO: when AppleClang supports execution policies, use them.
 */

#include "VideoDecoders.hpp"
#include "common/utils.hpp"
#include "common/panic.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <iterator>
#include <version>

// AppleClang doesn't support execution policies for now.
#if __cpp_lib_execution >= 201902L
#include <execution>
#define EXEC_UNSEQ_COMMA std::execution::unseq,
#else
#define EXEC_UNSEQ_COMMA
#endif

/** \brief An iterator that counts numbers. */
template<typename NUM, NUM INC = 1>
class CountingIterator
{
public:
    static_assert(INC != 0, "Increment must not be 0");

    using value_type = NUM;
    using reference = value_type&;
    using difference_type = ptrdiff_t;
    using pointer = value_type*;
    using iterator_category  = std::forward_iterator_tag;

    constexpr CountingIterator() noexcept : m_count{0} {}
    constexpr explicit CountingIterator(const value_type& min) noexcept : m_count{min} {}

    constexpr CountingIterator(const CountingIterator& other) noexcept = default;
    constexpr CountingIterator& operator=(const CountingIterator& other) noexcept = default;

    constexpr CountingIterator(CountingIterator&& other) noexcept = default;
    constexpr CountingIterator& operator=(CountingIterator&& other) noexcept = default;

    constexpr value_type& operator*() noexcept { return m_count; }

    constexpr CountingIterator& operator++() noexcept { m_count += INC; return *this; }
    constexpr CountingIterator& operator++(int) noexcept { m_count += INC; return *this; }

    constexpr bool operator==(const CountingIterator& other) noexcept { return m_count == other.m_count; }
    constexpr bool operator!=(const CountingIterator& other) noexcept { return m_count != other.m_count; }

private:
    value_type m_count;
};

namespace Video
{

/** \brief Decode a bitmap file line.
 * \tparam WIDTH The number of input pixels to decode.
 * \param dst Where the decoded line will be written to in ARGB.
 * \param dataA See below.
 * \param dataB See below.
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
template<uint16_t WIDTH>
uint16_t decodeBitmapLine(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept
{
    if(icm == ImageCodingMethod::DYUV)
        return decodeDYUVLine<WIDTH>(dst, dataB, initialDYUV);

    if(icm == ImageCodingMethod::RGB555)
    {
        if constexpr(WIDTH == 360 || WIDTH == 384)
        {
            return decodeRGB555Line<WIDTH>(dst, dataA, dataB);
        }
        else
        {
            panic("RGB555 is only usable in normal resolution");
        }
    }

    return decodeCLUTLine<WIDTH>(dst, dataB, CLUTTable, icm);
}
template uint16_t decodeBitmapLine<360>(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept;
template uint16_t decodeBitmapLine<384>(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept;
template uint16_t decodeBitmapLine<720>(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept;
template uint16_t decodeBitmapLine<768>(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept;

uint16_t decodeBitmapLine(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept
{
    if(icm == ImageCodingMethod::CLUT4)
        if(width == 720)
            return decodeBitmapLine<720>(dst, dataA, dataB, CLUTTable, initialDYUV, icm);
        else
            return decodeBitmapLine<768>(dst, dataA, dataB, CLUTTable, initialDYUV, icm);
    else if(width == 720 || width == 768)
        if(width == 720)
            return decodeBitmapLine<720>(dst, dataA, dataB, CLUTTable, initialDYUV, icm);
        else
            return decodeBitmapLine<768>(dst, dataA, dataB, CLUTTable, initialDYUV, icm);
    else
        if(width == 360)
            return decodeBitmapLine<360>(dst, dataA, dataB, CLUTTable, initialDYUV, icm);
        else
            return decodeBitmapLine<384>(dst, dataA, dataB, CLUTTable, initialDYUV, icm);
}

/** \brief Decode a Run-length file line.
 * \tparam WIDTH The number of input pixels to decode (must match \p RL3).
 * \tparam RL3 true when RL3 encoding is used, false for RL7.
 * \param dst Where the decoded line will be written in ARGB.
 * \param data The raw input data to be decoded.
 * \param CLUTTable The CLUT table to use.
 * \return The number of raw bytes read from data.
 *
 * This function is instantiated for
 * - RL3 double/high resolution
 * - RL7 normal resolution
 * - RL7 high resolution
 */
template<uint16_t WIDTH, bool RL3>
uint16_t decodeRunLengthLine(Pixel* dst, const uint8_t* data, const uint32_t* CLUTTable) noexcept
{
    uint16_t index = 0;

    if constexpr(RL3) // RL3
    {
        for(int x = 0; x < WIDTH;)
        {
            const uint8_t format = data[index++];
            const uint8_t color1 = bits<4, 6>(format);
            const uint8_t color2 = bits<0, 2>(format);

            uint16_t count = 0; // number of memcpy to do.
            if(bit<7>(format)) // run of pixels pairs
            {
                count = data[index++];
                if(count == 0)
                    count = (WIDTH - x) >> 1; // This is count in pixel pair, so half the width.
            }

            Pixel* pixels = &dst[x];
            x += 2;
            decodeCLUT(pixels, color1, CLUTTable);
            decodeCLUT(&pixels[1], color2, CLUTTable);
            for(int i = 1; i < count; i++) // Start at 1 because we already decoded the first pixel right before.
            {
                memcpy(&dst[x], pixels, sizeof(Pixel) * 2); // Copy pixel pair.
                x += 2;
            }
        }
    }
    else // RL7
    {
        // WIDTH, x and count are from the perspective of the source, it doesn't need to be treated differently based on
        // resolution because RL7 normal res simply duplicates the pixels, so only dst needs a different treatment.
        for(int x = 0; x < WIDTH;)
        {
            const uint8_t format = data[index++];
            const uint8_t color = bits<0, 6>(format);

            uint16_t count = 1; // number of memcpy to do.
            if(bit<7>(format)) // run of single pixels
            {
                count = data[index++];
                if(count == 0)
                    count = WIDTH - x;
            }
            x += count;

            Pixel* pixels = dst++;
            decodeCLUT(pixels, color, CLUTTable);
            if constexpr(WIDTH == 360 || WIDTH == 384) // Duplicate the first pixel in normal resolution.
            {
                memcpy(dst++, pixels, sizeof(Pixel));
            }

            for(int i = 1; i < count; i++) // Start at 1 because we already decoded the first pixel right before.
            {
                if constexpr(WIDTH == 720 || WIDTH == 768) // High resolution
                {
                    memcpy(dst++, pixels, sizeof(Pixel));
                }
                else // Normal resolution duplicates the pixel to fit the buffer.
                {
                    memcpy(dst, pixels, sizeof(Pixel) * 2);
                    dst += 2;
                }
            }
        }
    }

    return index;
}
template uint16_t decodeRunLengthLine<360, false>(Pixel* dst, const uint8_t* data, const uint32_t* CLUTTable) noexcept;
template uint16_t decodeRunLengthLine<384, false>(Pixel* dst, const uint8_t* data, const uint32_t* CLUTTable) noexcept;
template uint16_t decodeRunLengthLine<720, false>(Pixel* dst, const uint8_t* data, const uint32_t* CLUTTable) noexcept;
template uint16_t decodeRunLengthLine<768, false>(Pixel* dst, const uint8_t* data, const uint32_t* CLUTTable) noexcept;
template uint16_t decodeRunLengthLine<720, true>(Pixel* dst, const uint8_t* data, const uint32_t* CLUTTable) noexcept;
template uint16_t decodeRunLengthLine<768, true>(Pixel* dst, const uint8_t* data, const uint32_t* CLUTTable) noexcept;
template<> uint16_t decodeRunLengthLine<360, true>(Pixel* dst, const uint8_t* data, const uint32_t* CLUTTable) noexcept
    = delete; // ("RL3 source width is never normal resolution");
template<> uint16_t decodeRunLengthLine<384, true>(Pixel* dst, const uint8_t* data, const uint32_t* CLUTTable) noexcept
    = delete; // ("RL3 source width is never normal resolution");

uint16_t decodeRunLengthLine(Pixel* dst, const uint8_t* data, uint16_t width, const uint32_t* CLUTTable, bool is4BPP) noexcept
{
    if(is4BPP) // RL3
        if(width == 720)
            return decodeRunLengthLine<720, true>(dst, data, CLUTTable);
        else
            return decodeRunLengthLine<768, true>(dst, data, CLUTTable);
    else if(width == 720 || width == 768) // RL7 high
        if(width == 720)
            return decodeRunLengthLine<720, false>(dst, data, CLUTTable);
        else
            return decodeRunLengthLine<768, false>(dst, data, CLUTTable);
    else
        if(width == 360)
            return decodeRunLengthLine<360, false>(dst, data, CLUTTable);
        else
            return decodeRunLengthLine<384, false>(dst, data, CLUTTable);
}

/** \brief Decode a RGB555 line to ARGB using U32.
 * \tparam WIDTH The number of source pixels to decode.
 * \param dst Where the ARGB data will be written to.
 * \param dataA The plane A data (high order byte of the pixel).
 * \param dataB The plane B data (low order byte of the pixel).
 * \return The number of raw bytes read from each data source.
 *
 * The transparency bit is set in the alpha byte (0xFF when the bit is 1, 0 otherwise).
 */
template<uint16_t WIDTH>
uint16_t decodeRGB555Line(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB) noexcept
{
    for(uint16_t x = 0; x < WIDTH; x++)
    {
        // This version is highly vectorizable (to GCC) compared to manipulating dataA and dataB bytes independently.
        uint16_t rgb555 = static_cast<uint16_t>(*dataA++) << 8;
        rgb555 |= *dataB++;

        Pixel* pixel = dst++;
        pixel->a = (rgb555 & 0x8000) ? 0x80 : 0;
        pixel->r = rgb555 >> 7 & 0xF8;
        pixel->g = rgb555 >> 2 & 0xF8;
        pixel->b = rgb555 << 3 & 0xF8;
        memcpy(dst++, pixel, sizeof *dst); // RGB555 only supports normal res (GB Figure V.50).
    }

    return WIDTH;
}
template uint16_t decodeRGB555Line<360>(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB) noexcept;
template uint16_t decodeRGB555Line<384>(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB) noexcept;
template<> uint16_t decodeRGB555Line<720>(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB) noexcept = delete;
template<> uint16_t decodeRGB555Line<768>(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB) noexcept = delete;

static constexpr void matrixRGB(Pixel* pixel, const int Y, const uint8_t U, const uint8_t V) noexcept
{
    pixel->r = limu8(Y + matrixVToR[V]);
    pixel->g = limu8(Y - (matrixUToG[U] + matrixVToG[V]));
    pixel->b = limu8(Y + matrixUToB[U]);
}

/** \brief Decode a DYUV line to ARGB.
 * \tparam WIDTH The number of source pixels to decode.
 * \param dst Where the ARGB data will be written to.
 * \param dyuv The source DYUV data.
 * \param initialDYUV The initial value to be used by the DYUV decoder.
 * \return The number of raw bytes read from \p dyuv.
 */
template<uint16_t WIDTH>
uint16_t decodeDYUVLine(Pixel* dst, const uint8_t* dyuv, uint32_t initialDYUV) noexcept
{
    uint8_t py = bits<16, 23>(initialDYUV);
    uint8_t pu = bits<8, 15>(initialDYUV);
    uint8_t pv = initialDYUV;

    for(uint16_t index = 0; index < WIDTH; index += 2)
    {
        const uint8_t high = dyuv[index];
        const uint8_t low = dyuv[index + 1];

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

        Pixel* pixel1 = dst++;
        matrixRGB(pixel1, y1, u1, v1);
        if constexpr(WIDTH == 360 || WIDTH == 384)
        {
            memcpy(dst++, pixel1, sizeof *dst);
        }

        Pixel* pixel2 = dst++;
        matrixRGB(pixel2, y2, u2, v2);
        if constexpr(WIDTH == 360 || WIDTH == 384)
        {
            memcpy(dst++, pixel2, sizeof *dst);
        }
    }

    return WIDTH;
}
template uint16_t decodeDYUVLine<360>(Pixel* dst, const uint8_t* data, uint32_t initialDYUV) noexcept;
template uint16_t decodeDYUVLine<384>(Pixel* dst, const uint8_t* data, uint32_t initialDYUV) noexcept;
template uint16_t decodeDYUVLine<720>(Pixel* dst, const uint8_t* data, uint32_t initialDYUV) noexcept;
template uint16_t decodeDYUVLine<768>(Pixel* dst, const uint8_t* data, uint32_t initialDYUV) noexcept;

#define DYUV_PIXEL_INDEX(Y, U, V) (as<uint32_t>(Y) << 16 | as<uint32_t>(U) << 8 | V)

static std::vector<Pixel> generateDYUVPixelLUT() noexcept
{
    std::vector<Pixel> array{};
    array.resize(256 * 256 * 256);

    for(int y = 0; y < 256; ++y)
        for(int u = 0; u < 256; ++u)
            for(int v = 0; v < 256; ++v)
            {
                Pixel pixel{0};
                pixel.r = limu8(y + matrixVToR[v]);
                pixel.g = limu8(y - (matrixUToG[u] + matrixVToG[v]));
                pixel.b = limu8(y + matrixUToB[u]);
                array[DYUV_PIXEL_INDEX(y, u, v)] = pixel;
            }
    return array;
}

/** \brief LUT to convert the YUV values to RGB. Use #DYUV_PIXEL_INDEX macro to index this array. */
static const std::vector<Pixel> dyuvPixelLUT = generateDYUVPixelLUT();

/** \brief Decode a DYUV line to ARGB using a LUT.
 * \tparam WIDTH The number of source pixels to decode.
 * \param dst Where the ARGB data will be written to.
 * \param dyuv The source DYUV data.
 * \param initialDYUV The initial value to be used by the DYUV decoder.
 * \return The number of raw bytes read from \p dyuv.
 *
 * This is not a SIMD decoder because each pixel depends on the decoded value of the previous one.
 * However this is another approach that uses a pixel LUT to remove as much calculations as possible.
 */
template<uint16_t WIDTH>
uint16_t decodeDYUVLineLUT(Pixel* dst, const uint8_t* dyuv, uint32_t initialDYUV) noexcept
{
    uint8_t py = bits<16, 23>(initialDYUV);
    uint8_t pu = bits<8, 15>(initialDYUV);
    uint8_t pv = initialDYUV;

    for(uint16_t index = 0; index < WIDTH; index += 2)
    {
        const uint8_t high = dyuv[index];
        const uint8_t low = dyuv[index + 1];

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

        Pixel* pixel1 = dst++;
        *pixel1 = dyuvPixelLUT[DYUV_PIXEL_INDEX(y1, u1, v1)]; // Matrix RGB.
        if constexpr(WIDTH == 360 || WIDTH == 384)
        {
            memcpy(dst++, pixel1, sizeof(Pixel));
        }

        Pixel* pixel2 = dst++;
        *pixel2 = dyuvPixelLUT[DYUV_PIXEL_INDEX(y2, u2, v2)]; // Matrix RGB.
        if constexpr(WIDTH == 360 || WIDTH == 384)
        {
            memcpy(dst++, pixel2, sizeof(Pixel));
        }
    }

    return WIDTH;
}
template uint16_t decodeDYUVLineLUT<360>(Pixel* dst, const uint8_t* dyuv, uint32_t initialDYUV) noexcept;
template uint16_t decodeDYUVLineLUT<384>(Pixel* dst, const uint8_t* dyuv, uint32_t initialDYUV) noexcept;
template uint16_t decodeDYUVLineLUT<720>(Pixel* dst, const uint8_t* dyuv, uint32_t initialDYUV) noexcept;
template uint16_t decodeDYUVLineLUT<768>(Pixel* dst, const uint8_t* dyuv, uint32_t initialDYUV) noexcept;

/** \brief Decode a CLUT line to ARGB.
 * \tparam WIDTH The number of source pixels to decode.
 * \param dst Where the ARGB data will be written to.
 * \param data The source CLUT data.
 * \param CLUTTable The CLUT to use when decoding.
 * \param icm The coding method (non-CLUT values are ignored and treated as CLUT7).
 * \return The number of raw bytes read from data.
 *
 * If \p icm is ImageCodingMethod::CLUT77 or the video plane is plane B, then sent `&CLUT[128]` as the \p CLUTTable.
 */
template<uint16_t WIDTH>
uint16_t decodeCLUTLine(Pixel* dst, const uint8_t* data, const uint32_t* CLUTTable, ImageCodingMethod icm) noexcept
{
    if(icm == ImageCodingMethod::CLUT4)
    {
        CountingIterator<int> it{0};
        std::for_each_n(EXEC_UNSEQ_COMMA it, WIDTH >> 1, [dst, data, CLUTTable] (int index) {
            const uint8_t d = data[index];
            const uint8_t color1 = bits<4, 7>(d);
            const uint8_t color2 = bits<0, 3>(d);
            index *= 2;
            decodeCLUT(&dst[index], color1, CLUTTable);
            decodeCLUT(&dst[index + 1], color2, CLUTTable);
            // CLUT4 is always double/high resolution.
        });

        return WIDTH >> 1;
    }
    else
    {
        const uint8_t colorMask = icm == ImageCodingMethod::CLUT8 ? 0xFF : 0x7F;

        std::for_each_n(EXEC_UNSEQ_COMMA data, WIDTH, [dst, CLUTTable, colorMask] (uint8_t color) mutable {
            color &= colorMask;
            Pixel* pixel = dst++;
            decodeCLUT(pixel, color, CLUTTable);
            if constexpr(WIDTH == 360 || WIDTH == 384)
            {
                memcpy(dst++, pixel, sizeof *dst);
            }
        });

        return WIDTH;
    }
}
template uint16_t decodeCLUTLine<360>(Pixel* dst, const uint8_t* data, const uint32_t* CLUTTable, ImageCodingMethod icm) noexcept;
template uint16_t decodeCLUTLine<384>(Pixel* dst, const uint8_t* data, const uint32_t* CLUTTable, ImageCodingMethod icm) noexcept;
template uint16_t decodeCLUTLine<720>(Pixel* dst, const uint8_t* data, const uint32_t* CLUTTable, ImageCodingMethod icm) noexcept;
template uint16_t decodeCLUTLine<768>(Pixel* dst, const uint8_t* data, const uint32_t* CLUTTable, ImageCodingMethod icm) noexcept;

/** \brief Copy the ARGB pixels to an ARGB plane.
 *
 * \param dst The destination ARGB plane.
 * \param dstWidth The width of the destination plane.
 * \param dstHeight The height of the destination plane.
 * \param src The source ARGB plane.
 * \param srcWidth The width of the source plane.
 * \param srcHeight The width of the source plane.
 * \param xOffset The x offset in pixels where the paste will occur on dst.
 * \param yOffset The y offset in pixels where the paste will occur on dst.
 *
 * If the source does not fit in the destination, only the pixels that fit in the destination are copied.
 */
void paste(Pixel* dst, const uint16_t dstWidth, const uint16_t dstHeight, const Pixel* src, const uint16_t srcWidth, const uint16_t srcHeight, const uint16_t xOffset, const uint16_t yOffset)
{
    for(uint16_t dy = yOffset, sy = 0; dy < dstHeight && sy < srcHeight; ++dy, ++sy)
    {
              Pixel* dstRow = dst + dstWidth * dy;
        const Pixel* srcRow = src + srcWidth * sy;
        for(uint16_t dx = xOffset, sx = 0; dx < dstWidth && sx < srcWidth; ++dx, ++sx)
        {
            if(srcRow[sx].a != 0) // Alpha is either 0 or 255.
            {
                dstRow[dx] = srcRow[sx];
            }
        }
    }
}

} // namespace Video
