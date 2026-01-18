#ifndef CDI_VIDEO_VIDEOCOMMON_HPP
#define CDI_VIDEO_VIDEOCOMMON_HPP

#include "Pixel.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace Video
{

/** \brief An image plane. */
enum ImagePlane : uint8_t
{
    A = 0,
    B = 1,
};

#define ICM(method) Video::ImageCodingMethod::method

enum class ImageCodingMethod
{
    OFF,
    RGB555, /**< Plane B only. */
    DYUV,
    CLUT8, /**< Plane A only. */
    CLUT7,
    CLUT77, /**< Plane A only. */
    CLUT4,
};

enum class ControlArea
{
    ICA1,
    DCA1,
    ICA2,
    DCA2,
};

/** \brief ARGB plane that uses uint32_t to store each pixel. */
class Plane : public std::vector<Pixel>
{
public:
    static constexpr size_t MAX_WIDTH     = 768;
    static constexpr size_t MAX_HEIGHT    = 560;
    static constexpr size_t CURSOR_WIDTH  = 16;
    static constexpr size_t CURSOR_HEIGHT = 16;

    static constexpr size_t MAX_SIZE  = MAX_WIDTH * MAX_HEIGHT;
    static constexpr size_t CURSOR_SIZE = CURSOR_WIDTH * CURSOR_HEIGHT;

    size_t m_width; /**< Width of the plane. */
    size_t m_height; /**< Height of the plane. */

    constexpr explicit Plane(const uint16_t w = 0, const uint16_t h = 0, const size_t size = MAX_SIZE)
        : std::vector<Pixel>(size, 0), m_width(w), m_height(h)
    {}

    /** \brief Returns a const pointer to the beginning of the given line (starting at 0). */
    constexpr const Pixel* GetLinePointer(const size_t line) const noexcept { return data() + line * m_width; }
    /** \brief Returns a pointer to the beginning of the given line (starting at 0). */
    constexpr Pixel* GetLinePointer(const size_t line) noexcept { return data() + line * m_width; }

    /** \brief Returns the number of pixels used by the plane. */
    constexpr size_t PixelCount() const noexcept { return m_width * m_height; }

    /** \brief Returns a span of the actual pixels used for this resolution. */
    constexpr std::span<const Pixel> GetSpan() const noexcept { return {data(), PixelCount()}; }
};

} // namespace Video

#endif // CDI_VIDEO_VIDEOCOMMON_HPP
