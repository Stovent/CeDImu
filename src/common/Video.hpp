#ifndef VIDEO_HPP
#define VIDEO_HPP

#include <cstdint>

namespace Video
{

enum CodingInformation
{
    ascf       = 0b10000000, // Application Specific Coding Flag
    eolf       = 0b01000000, // Even/Odd Lines Flag
    resolution = 0b00110000,
    coding     = 0b00001111,
};

extern const uint8_t dequantizer[16];
extern uint32_t CLUT[256];

uint16_t DecodeBitmapLine(uint8_t* input, uint8_t* output, const uint16_t width, const uint8_t codingMethod);
uint16_t DecodeRunLengthLine(uint8_t* line, const uint16_t width, const uint32_t* CLUTTable, const uint8_t* data, const bool cm);

// Real-Time Decoders (set pixels in RGB format)
uint8_t DecodeRGB555(const uint16_t pixel, uint8_t pixels[3]);
void DecodeDYUV(const uint16_t pixel, uint8_t pixels[6], const uint32_t previous);
void DecodeCLUT(const uint8_t pixel, uint8_t pixels[3], const uint32_t* CLUTTable);

void SplitARGB(const uint8_t* argb, const size_t argbLength, uint8_t* alpha, uint8_t* rgb);
void Paste(uint8_t* dst, const uint16_t dstWidth, const uint16_t dstHeight, const uint8_t* src, const uint16_t srcWidth, const uint16_t srcHeight, const uint16_t xOffset = 0, const uint16_t yOffset = 0);

} // namespace Video

#endif // VIDEO_HPP
