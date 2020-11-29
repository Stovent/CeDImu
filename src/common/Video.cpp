#include "Video.hpp"

#include <wx/msgdlg.h>

namespace Video
{

constexpr uint8_t dequantizer[16] = {0, 1, 4, 9, 16, 27, 44, 79, 128, 177, 212, 229, 240, 247, 252, 255};
uint8_t CLUT[256 * 3] = {0};

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
            DecodeCLUT(input[index] & 0xF0, &output[x++ * 3], codingMethod);
            DecodeCLUT(input[index++] << 4, &output[x++ * 3], codingMethod);
        }
        else // CLUT
        {
            DecodeCLUT(input[index++], &output[x++ * 3], codingMethod);
        }
    }
    return index;
}

uint16_t DecodeRunLengthLine(uint8_t* input, uint8_t* output, const uint16_t width, bool cm)
{
    uint16_t index = 0;

    for(int x = 0; x < width;)
    {
        uint8_t format = input[index++];
        if(cm) // RL3
        {
            uint8_t color1 =  format & 0x70;
            uint8_t color2 = (format & 0x07) << 4;
            uint16_t count = 1;
            if(format & 0x80) // run of pixels pairs
                count = input[index++];

            if(count == 0)
                count = width - x;

            for(int i = 0; i < count; i++)
            {
                DecodeCLUT(color1, &output[x++ * 3], 0);
                DecodeCLUT(color2, &output[x++ * 3], 0);
            }
        }
        else // RL7
        {
            uint8_t color =  format & 0x7F;
            uint16_t count = 1;
            if(format & 0x80) // run of pixels pairs
                count = input[index++];

            if(count == 0)
                count = width - x;

            for(int i = 0; i < count; i++)
                DecodeCLUT(color, &output[x++ * 3], 1);
        }
    }
    return index;
}

uint8_t DecodeRGB555(const uint16_t pixel, uint8_t pixels[3])
{
    pixels[0] = (pixel & 0x7C00) >> 7;
    pixels[1] = (pixel & 0x03E0) >> 2;
    pixels[2] = (pixel & 0x001F) << 3;
    return (pixel & 0x8000) ? 0xFF : 0;
}

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
    u2 = u1 = (pu + dequantizer[u1]) % 256; // u2 should be interpolated with the next u1
    v2 = v1 = (pv + dequantizer[v1]) % 256; // v2 should be interpolated with the next v1
    y2 = (y1 + dequantizer[y1]) % 256;

    pixels[0] = y1 + (v1 - 128) * 1.371; // R1
    pixels[2] = y1 + (u1 - 128) * 1.733; // B1
    pixels[1] = (y1 - 0.299 * pixels[0] - 0.114 * pixels[2]) / 0.587; // G1
    pixels[3] = y2 + (v2 - 128) * 1.371; // R2
    pixels[5] = y2 + (u2 - 128) * 1.733; // B2
    pixels[4] = (y2 - 0.299 * pixels[3] - 0.114 * pixels[5]) / 0.587; // G2
}

void DecodeCLUT(uint8_t pixel, uint8_t pixels[3], const uint8_t CLUTType)
{
    if(CLUTType == 1)
    {
        pixel &= 0x7F;
    }
    else if(CLUTType == 0)
    {
        pixel >>= 4;
    }
    else if(CLUTType != 2)
    {
        wxMessageBox("WARNING: wrong CLUT type in file");
    }

    pixels[0] = CLUT[pixel * 3];
    pixels[1] = CLUT[pixel * 3 + 1];
    pixels[2] = CLUT[pixel * 3 + 2];
}

} // namespace Video
