#ifndef CDI_COMMON_VIDEO_HPP
#define CDI_COMMON_VIDEO_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Video
{

enum CodingInformation : uint8_t
{
    ascf       = 0b10000000, // Application Specific Coding Flag
    eolf       = 0b01000000, // Even/Odd Lines Flag
    resolution = 0b00110000,
    coding     = 0b00001111,
};

#define ICM(method) Video::ImageCodingMethod::method

enum class ImageCodingMethod
{
    OFF,
    CLUT8,
    CLUT7,
    CLUT77,
    CLUT4,
    DYUV,
    RGB555,
};

enum class ControlArea
{
    ICA1,
    DCA1,
    ICA2,
    DCA2,
};

/** \struct Plane
 * \brief Represent a video plane.
 */
struct Plane : public std::vector<uint8_t>
{
    static constexpr size_t MAX_WIDTH     = 768;
    static constexpr size_t MAX_HEIGHT    = 560;
    static constexpr size_t CURSOR_WIDTH  = 16;
    static constexpr size_t CURSOR_HEIGHT = 16;

    static constexpr size_t RGB_SIZE   = MAX_WIDTH * MAX_HEIGHT * 3;
    static constexpr size_t ARGB_SIZE  = MAX_WIDTH * MAX_HEIGHT * 4;
    static constexpr size_t CURSOR_ARGB_SIZE = CURSOR_WIDTH * CURSOR_HEIGHT * 4;

    uint16_t width; /**< Width of the plane. */
    uint16_t height; /**< Height of the plane. */

    explicit Plane(const size_t sz = ARGB_SIZE, uint16_t w = 0, uint16_t h = 0) : std::vector<uint8_t>(sz, 0), width(w), height(h) {}
    const uint8_t* operator()(size_t line, size_t pixelSize) const { return data() + line * width * pixelSize; }
    uint8_t* operator()(size_t line, size_t pixelSize) { return data() + line * width * pixelSize; }
};

extern const uint8_t dequantizer[16];
extern uint32_t CLUT[256];

uint16_t decodeBitmapLine(uint8_t* line, const uint16_t width, const uint8_t* dataA, const uint8_t* dataB, const uint32_t* CLUTTable, const uint32_t initialDYUV, const ImageCodingMethod codingMethod);
uint16_t decodeRunLengthLine(uint8_t* line, const uint16_t width, const uint8_t* data, const uint32_t* CLUTTable, const bool cm);

// Real-Time Decoders (set pixels in ARGB format)
void decodeRGB555(const uint16_t pixel, uint8_t pixels[4]);
void decodeDYUV(const uint16_t pixel, uint8_t pixels[8], uint32_t& previous);
void decodeCLUT(const uint8_t pixel, uint8_t pixels[4], const uint32_t* CLUTTable);

void splitARGB(const uint8_t* argb, const size_t argbLength, uint8_t* alpha, uint8_t* rgb);
void paste(uint8_t* dst, const uint16_t dstWidth, const uint16_t dstHeight, const uint8_t* src, const uint16_t srcWidth, const uint16_t srcHeight, const uint16_t xOffset = 0, const uint16_t yOffset = 0);

} // namespace Video

#endif // CDI_COMMON_VIDEO_HPP
