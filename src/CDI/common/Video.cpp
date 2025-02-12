#include "Video.hpp"
#include "utils.hpp"

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
uint16_t decodeBitmapLine(uint8_t* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept
{
    if(icm == ImageCodingMethod::DYUV)
        return decodeDYUVLine(dst, dataB, width, initialDYUV);

    if(icm == ImageCodingMethod::RGB555)
        return decodeRGB555Line(dst, dataA, dataB, width);

    return decodeCLUTLine(dst, dataB, width, CLUTTable, icm);
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
uint16_t decodeRunLengthLine(uint8_t* dst, const uint8_t* data, uint16_t width, const uint32_t* CLUTTable, bool is4BPP) noexcept
{
    uint16_t index = 0;

    if(is4BPP) // RL3
    {
        for(int x = 0; x < width;)
        {
            const uint8_t format = data[index++];
            const uint8_t color1 = bits<4, 6>(format);
            const uint8_t color2 = bits<0, 2>(format);
            uint16_t count = 1;

            if(bit<7>(format)) // run of pixels pairs
            {
                count = data[index++];
                if(count == 0)
                    count = width - x;
            }

            for(int i = 0; i < count; i++)
            {
                decodeCLUT(color1, &dst[x++ * 4], CLUTTable);
                decodeCLUT(color2, &dst[x++ * 4], CLUTTable);
            }
        }
    }
    else // RL7
    {
        for(int x = 0; x < width;)
        {
            const uint8_t format = data[index++];
            const uint8_t color = bits<0, 6>(format);
            uint16_t count = 1;

            if(bit<7>(format)) // run of single pixels
            {
                count = data[index++];
                if(count == 0)
                    count = width - x;
            }

            for(int i = 0; i < count; i++)
            {
                decodeCLUT(color, &dst[x++ * 4], CLUTTable);
            }
        }
    }

    return index;
}

/** \brief Decode a RGB555 line to ARGB.
 * \param dst Where the ARGB data will be written to.
 * \param dataA The plane A data (high order byte of the pixel).
 * \param dataB The plane B data (low order byte of the pixel).
 * \param width Width of the line in pixels.
 * \return The number of raw bytes read from each data source.
 */
uint16_t decodeRGB555Line(uint8_t* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width) noexcept
{
    uint16_t index = 0;

    for(uint16_t x = 0; x < width; x++)
    {
        uint16_t pixel = as<uint16_t>(dataA[index]) << 8;
        pixel |= dataB[index++];
        decodeRGB555(pixel, &dst[x * 4]);
    }

    return index;
}

/** \brief Decode a DYUV line to ARGB.
 * \param dst Where the ARGB data will be written to.
 * \param dyuv The source DYUV data.
 * \param width Width of the line in pixels.
 * \param initialDYUV The initial value to be used by the DYUV decoder.
 * \return The number of raw bytes read from \p dyuv.
 */
uint16_t decodeDYUVLine(uint8_t* dst, const uint8_t* dyuv, uint16_t width, uint16_t initialDYUV) noexcept
{
    uint16_t index = 0;
    uint32_t previous = initialDYUV;

    for(uint16_t x = 0; x < width; x += 2)
    {
        uint16_t pixel = as<uint16_t>(dyuv[index++]) << 8;
        pixel |= dyuv[index++];
        decodeDYUV(pixel, &dst[x * 4], previous);
    }

    return index;
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
uint16_t decodeCLUTLine(uint8_t* dst, const uint8_t* data, uint16_t width, const uint32_t* CLUTTable, ImageCodingMethod icm) noexcept
{
    uint16_t index = 0;

    if(icm == ImageCodingMethod::CLUT4)
    {
        for(uint16_t x = 0; x < width;)
        {
            const uint8_t color1 = bits<4, 7>(data[index]);
            const uint8_t color2 = bits<0, 3>(data[index++]);
            decodeCLUT(color1, &dst[x++ * 4], CLUTTable);
            decodeCLUT(color2, &dst[x++ * 4], CLUTTable);
        }
    }
    else
    {
        const uint8_t colorMask = icm == ImageCodingMethod::CLUT8 ? 0xFF : 0x7F;
        for(uint16_t x = 0; x < width;)
        {
            const uint8_t color = data[index++] & colorMask;
            decodeCLUT(color, &dst[x++ * 4], CLUTTable);
        }
    }

    return index;
}

/** \brief Convert RGB555 to ARGB.
 *
 * \param pixel The pixel to decode.
 * \param pixels Where the pixel will be written to. pixels[0] = transparency, pixels[1] = red, pixels[2] = green, pixels[3] = blue.
 */
void decodeRGB555(const uint16_t pixel, uint8_t pixels[4]) noexcept
{
    pixels[0] = (pixel & 0x8000) ? 0xFF : 0;
    pixels[1] = pixel >> 7 & 0xF8;
    pixels[2] = pixel >> 2 & 0xF8;
    pixels[3] = pixel << 3 & 0xF8;
}

static constexpr void matrixRGB(const int Y, const uint8_t U, const uint8_t V, uint8_t pixels[4]) noexcept
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
void decodeDYUV(const uint16_t pixel, uint8_t pixels[8], uint32_t& previous) noexcept
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

    matrixRGB(y1, u1, v1, pixels);
    matrixRGB(y2, u2, v2, &pixels[4]);
}

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
void decodeCLUT(const uint8_t pixel, uint8_t pixels[4], const uint32_t* CLUTTable) noexcept
{
    pixels[0] = 0xFF;
    pixels[1] = bits<16, 23>(CLUTTable[pixel]);
    pixels[2] = bits<8, 15>(CLUTTable[pixel]);
    pixels[3] = CLUTTable[pixel];
}

/** \brief Split ARGB data into different alpha and RGB channels.
 *
 * \param argb The input ARGB data.
 * \param argbLength the number of ARGB bytes (so 4 times the number of pixels).
 * \param alpha The destination alpha channel.
 * \param rgb The destination RGB channel.
 *
 * A nullptr in a channel will not write to it.
 * TODO: This should not be a CDI function, but a GUI-specific one.
 */
void splitARGB(const uint8_t* argb, const size_t argbLength, uint8_t* alpha, uint8_t* rgb)
{
    if(alpha != nullptr && rgb != nullptr)
    {
        for(size_t i = 0; i < argbLength; i += 4)
        {
            *alpha++ = *argb++;
            *rgb++ = *argb++;
            *rgb++ = *argb++;
            *rgb++ = *argb++;
        }
    }
    else if(alpha != nullptr)
    {
        for(size_t i = 0; i < argbLength;)
        {
            *alpha++ = argb[i++];
            i += 3;
        }
    }
    else if(rgb != nullptr)
    {
        for(size_t i = 0; i < argbLength; i += 4)
        {
            argb++;
            *rgb++ = *argb++;
            *rgb++ = *argb++;
            *rgb++ = *argb++;
        }
    }
}

/** \brief Copy the ARGB pixels to an RGB plane.
 *
 * \param dst The destination RGB plane.
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
void paste(uint8_t* dst, const uint16_t dstWidth, const uint16_t dstHeight, const uint8_t* src, const uint16_t srcWidth, const uint16_t srcHeight, const uint16_t xOffset, const uint16_t yOffset)
{
    for(uint16_t dy = yOffset, sy = 0; dy < dstHeight && sy < srcHeight; dy++, sy++)
    {
              uint8_t* dstRow = dst + dstWidth * 3 * dy;
        const uint8_t* srcRow = src + srcWidth * 4 * sy;
        for(uint16_t dx = xOffset, sx = 0; dx < dstWidth && sx < srcWidth; dx++, sx++)
        {
            dstRow[dx * 3]     = (srcRow[sx * 4 + 1] * srcRow[sx * 4] + dstRow[dx * 3]     * (255 - srcRow[sx * 4])) / 255;
            dstRow[dx * 3 + 1] = (srcRow[sx * 4 + 2] * srcRow[sx * 4] + dstRow[dx * 3 + 1] * (255 - srcRow[sx * 4])) / 255;
            dstRow[dx * 3 + 2] = (srcRow[sx * 4 + 3] * srcRow[sx * 4] + dstRow[dx * 3 + 2] * (255 - srcRow[sx * 4])) / 255;
        }
    }
}

} // namespace Video
