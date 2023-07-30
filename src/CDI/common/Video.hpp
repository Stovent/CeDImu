#ifndef CDI_COMMON_VIDEO_HPP
#define CDI_COMMON_VIDEO_HPP

#include <array>
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

/** \brief Helper class representing a video plane.
 */
struct Plane : public std::vector<uint8_t>
{
    static constexpr size_t MAX_WIDTH     = 768;
    static constexpr size_t MAX_HEIGHT    = 560;
    static constexpr size_t CURSOR_WIDTH  = 16;
    static constexpr size_t CURSOR_HEIGHT = 16;

    static constexpr size_t RGB_MAX_SIZE   = MAX_WIDTH * MAX_HEIGHT * 3;
    static constexpr size_t ARGB_MAX_SIZE  = MAX_WIDTH * MAX_HEIGHT * 4;
    static constexpr size_t CURSOR_ARGB_SIZE = CURSOR_WIDTH * CURSOR_HEIGHT * 4;

    uint16_t m_width; /**< Width of the plane. */
    uint16_t m_height; /**< Height of the plane. */
    uint16_t m_bpp; /**< Bytes per pixels. */

    Plane() = delete;
    Plane(const uint16_t bpp, const uint16_t w = 0, const uint16_t h = 0, const size_t size = ARGB_MAX_SIZE)
        : std::vector<uint8_t>(size, 0), m_width(w), m_height(h), m_bpp(bpp)
    {}

    /** \brief Returns a const pointer to the beginning of the given line. */
    const uint8_t* operator()(const size_t line) const { return data() + line * m_width * m_bpp; }
    /** \brief Returns a pointer to the beginning of the given line. */
    uint8_t* operator()(const size_t line) { return data() + line * m_width * m_bpp; }
};

extern const std::array<uint8_t, 16> dequantizer;
extern uint32_t CLUT[256];

// Display file decoders.
uint16_t decodeBitmapLine(uint8_t* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm);
uint16_t decodeRunLengthLine(uint8_t* dst, const uint8_t* data, uint16_t width, const uint32_t* CLUTTable, bool is4BPP);

uint16_t decodeRGB555Line(uint8_t* dst, const uint8_t* dataA, const uint8_t* dataB, uint16_t width);
uint16_t decodeDYUVLine(uint8_t* dst, const uint8_t* data, uint16_t width, uint16_t initialDYUV);
uint16_t decodeCLUTLine(uint8_t* dst, const uint8_t* data, uint16_t width, const uint32_t* CLUTTable, ImageCodingMethod icm);

// Real-Time Decoders (set pixels in ARGB format)
void decodeRGB555(const uint16_t pixel, uint8_t pixels[4]);
void decodeDYUV(const uint16_t pixel, uint8_t pixels[8], uint32_t& previous);
void decodeCLUT(const uint8_t pixel, uint8_t pixels[4], const uint32_t* CLUTTable);

void splitARGB(const uint8_t* argb, const size_t argbLength, uint8_t* alpha, uint8_t* rgb);
void paste(uint8_t* dst, const uint16_t dstWidth, const uint16_t dstHeight, const uint8_t* src, const uint16_t srcWidth, const uint16_t srcHeight, const uint16_t xOffset = 0, const uint16_t yOffset = 0);

} // namespace Video

#endif // CDI_COMMON_VIDEO_HPP
