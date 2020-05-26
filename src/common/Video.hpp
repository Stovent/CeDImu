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
extern uint8_t CLUT[256 * 3];

uint16_t DecodeBitmapLine(uint8_t* input, uint8_t* output, const uint16_t width, const uint8_t codingMethod);
uint16_t DecodeRunLengthLine(uint8_t* input, uint8_t* output, const uint16_t width, bool cm);

// Real-Time Decoders (set pixels in RGB format)
uint8_t DecodeRGB555(const uint16_t pixel, uint8_t pixels[3]); // returns the alpha byte
void DecodeDYUV(const uint16_t pixel, uint8_t pixels[6], const uint32_t previous);
void DecodeCLUT(uint8_t pixel, uint8_t pixels[3], const uint8_t CLUTType);

} // namespace Video

#endif // VIDEO_HPP
