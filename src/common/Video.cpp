#include "Video.hpp"

#include <wx/msgdlg.h>

namespace Video
{

constexpr uint8_t dequantizer[16] = {0, 1, 4, 9, 16, 27, 44, 79, 128, 177, 212, 229, 240, 247, 252, 255};
uint32_t CLUT[256] = {0};

uint16_t DecodeBitmapLine(uint8_t* input, uint8_t* output, const uint16_t width, const uint8_t codingMethod)
{
    uint16_t index = 0;
    uint32_t previous = 0;

    for(uint16_t x = 0; x < width;)
    {
        if(codingMethod == 5)
        {
            DecodeDYUV(input[index++], &output[x * 3], previous);
            previous = 0;
            previous |= output[x * 3] << 16;
            previous |= output[x * 3 + 1] << 8;
            previous |= output[x++ * 3 + 2];
        }
        else if(codingMethod == 0)
        {
            DecodeCLUT(input[index] & 0xF0, &output[x++ * 3], Video::CLUT);
            DecodeCLUT(input[index++] << 4, &output[x++ * 3], Video::CLUT);
        }
        else // CLUT
        {
            DecodeCLUT(input[index++], &output[x++ * 3], Video::CLUT);
        }
    }
    return index;
}

/** \brief Decode a Run-length file line.
 *
 * \param line Where the decoded line will be written in ARGB.
 * \param width The width of the line.
 * \param CLUTTable The CLUT table to use.
 * \param data The raw input data to be decoded.
 * \param cm true for RL3, false for RL7.
 * \return The number of raw bytes read from data.
*/
uint16_t DecodeRunLengthLine(uint8_t* line, const uint16_t width, const uint32_t* CLUTTable, const uint8_t* data, const bool cm)
{
    uint16_t index = 0;

    if(cm) // RL3
    {
        for(int x = 0; x < width;)
        {
            const uint8_t format = data[index++];
            const uint8_t color1 = format >> 4 & 0x07;
            const uint8_t color2 = format & 0x07;
            uint16_t count = 1;
            if(format & 0x80) // run of pixels pairs
            {
                count = data[index++];;
                if(count == 0)
                    count = width - x;
            }

            for(int i = 0; i < count; i++)
            {
                line[x * 4] = 0xFF;
                Video::DecodeCLUT(color1, &line[x++ * 4 + 1], CLUTTable);
                line[x * 4] = 0xFF;
                Video::DecodeCLUT(color2, &line[x++ * 4 + 1], CLUTTable);
            }
        }
    }
    else // RL7
    {
        for(int x = 0; x < width;)
        {
            const uint8_t format = data[index++];
            const uint8_t color = format & 0x7F;
            uint16_t count = 1;
            if(format & 0x80) // run of single pixels
            {
                count = data[index++];;
                if(count == 0)
                    count = width - x;
            }

            for(int i = 0; i < count; i++)
            {
                line[x * 4] = 0xFF;
                Video::DecodeCLUT(color, &line[x++ * 4 + 1], CLUTTable);
            }
        }
    }

    return index;
}

/** \brief Convert RGB555 to RGB888.
 *
 * \param pixel The pixel to decode.
 * \param pixels Where the pixels will be written to. pixels[0] = red, pixels[1] = green, pixels[2] = blue.
 * \return The alpha byte.
*/
uint8_t DecodeRGB555(const uint16_t pixel, uint8_t pixels[3])
{
    pixels[0] = pixel >> 7 & 0xF8;
    pixels[1] = pixel >> 2 & 0xF8;
    pixels[2] = pixel << 3 & 0xF8;
    return (pixel & 0x8000) ? 0xFF : 0;
}

/** \brief Convert DYUV to RGB888.
 *
 * \param pixel The pixel to decode.
 * \param pixels Where the pixels will be written to.  pixels[0,3] = red, pixels[1,4] = green, pixels[2,5] = blue.
 * \param previous The previous pixel colors.
 *
 * TODO: interpolate u2 and v2 with the next u1 and v1.
*/
void DecodeDYUV(const uint16_t pixel, uint8_t pixels[6], const uint32_t previous)
{
    uint8_t y1, u1, v1, y2, u2, v2, py, pu, pv;
    u1 = (pixel & 0xF000) >> 12;
    y1 = (pixel & 0x0F00) >> 8;
    v1 = (pixel & 0x00F0) >> 4;
    y2 =  pixel & 0x000F;
    py = previous >> 16;
    pu = previous >> 8;
    pv = previous;

    y1 = (py + dequantizer[y1]) % 256;
    u2 = u1 = (pu + dequantizer[u1]) % 256; // Interpolation should be done after the line is drawn.
    v2 = v1 = (pv + dequantizer[v1]) % 256; // Interpolation should be done after the line is drawn.
    y2 = (y1 + dequantizer[y1]) % 256;

    pixels[0] = y1 + (v1 - 128) * 1.371; // R1
    pixels[2] = y1 + (u1 - 128) * 1.733; // B1
    pixels[1] = (y1 - 0.299 * pixels[0] - 0.114 * pixels[2]) / 0.587; // G1
    pixels[3] = y2 + (v2 - 128) * 1.371; // R2
    pixels[5] = y2 + (u2 - 128) * 1.733; // B2
    pixels[4] = (y2 - 0.299 * pixels[3] - 0.114 * pixels[5]) / 0.587; // G2
}

/** \brief Convert CLUT color to RGB888.
 *
 * \param pixel The CLUT address (must be in the lower bits).
 * \param pixels Where the pixels will be written to. pixels[0] = red, pixels[1] = green, pixels[2] = blue.
 * \param CLUTTable The CLUT table to use.
 *
 * pixel must have an offset of 128 when:
 * - plane B is the plane being drawn.
 * - bit 22 of register ImageCodingMethod is set for CLUT7+7.
*/
void DecodeCLUT(const uint8_t pixel, uint8_t pixels[3], const uint32_t* CLUTTable)
{
    pixels[0] = CLUTTable[pixel] >> 16;
    pixels[1] = CLUTTable[pixel] >> 8;
    pixels[2] = CLUTTable[pixel];
}

/** \brief Split ARGB data into different alpha and RGB channels.
 *
 * \param argb The input ARGB data.
 * \param argbLength the number of ARGB bytes (so 4 times the number of pixels).
 * \param alpha The destination alpha channel.
 * \param rgb The destination RGB channel.
 *
 * A nullptr in a channel will not write to it.
*/
void SplitARGB(const uint8_t* argb, const size_t argbLength, uint8_t* alpha, uint8_t* rgb)
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
void Paste(uint8_t* dst, const uint16_t dstWidth, const uint16_t dstHeight, const uint8_t* src, const uint16_t srcWidth, const uint16_t srcHeight, const uint16_t xOffset, const uint16_t yOffset)
{
    for(uint16_t dy = yOffset, sy = 0; dy < dstHeight && sy < srcHeight; dy++, sy++)
    {
              uint8_t* dstRow = dst + dstWidth * 3 * dy;
        const uint8_t* srcRow = src + srcWidth * 4 * sy;
        for(uint16_t dx = xOffset, sx = 0; dx < dstWidth && sx < srcWidth; dx++, sx++)
        {
            dstRow[dx * 3]     = srcRow[sx * 4 + 1] + dstRow[dx * 3]     * (1 - srcRow[sx * 4]);
            dstRow[dx * 3 + 1] = srcRow[sx * 4 + 2] + dstRow[dx * 3 + 1] * (1 - srcRow[sx * 4]);
            dstRow[dx * 3 + 2] = srcRow[sx * 4 + 3] + dstRow[dx * 3 + 2] * (1 - srcRow[sx * 4]);
        }
    }
}

} // namespace Video
