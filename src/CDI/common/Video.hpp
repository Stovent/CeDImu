#ifndef VIDEO_HPP
#define VIDEO_HPP

#include <cstddef>
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

uint16_t decodeBitmapLine(uint8_t* line, const uint16_t width, const uint8_t* dataA, const uint8_t* dataB, const uint32_t* CLUTTable, const uint32_t initialDYUV, const uint8_t codingMethod);
uint16_t decodeRunLengthLine(uint8_t* line, const uint16_t width, const uint8_t* data, const uint32_t* CLUTTable, const bool cm);

// Real-Time Decoders (set pixels in ARGB format)
void decodeRGB555(const uint16_t pixel, uint8_t pixels[4]);
void decodeDYUV(const uint16_t pixel, uint8_t pixels[8], const uint32_t previous);
void decodeCLUT(const uint8_t pixel, uint8_t pixels[4], const uint32_t* CLUTTable);

void splitARGB(const uint8_t* argb, const size_t argbLength, uint8_t* alpha, uint8_t* rgb);
void paste(uint8_t* dst, const uint16_t dstWidth, const uint16_t dstHeight, const uint8_t* src, const uint16_t srcWidth, const uint16_t srcHeight, const uint16_t xOffset = 0, const uint16_t yOffset = 0);

} // namespace Video

#endif // VIDEO_HPP
