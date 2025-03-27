#ifndef CDI_VIDEO_RENDERERSOFTWAREU32_HPP
#define CDI_VIDEO_RENDERERSOFTWAREU32_HPP

#include "Renderer.hpp"
#include "../common/VideoU32.hpp"

namespace Video
{

/** \brief CD-i video renderer implementation using only C++ code and uint32_t to represent pixels.
 */
class RendererSoftwareU32 final : public Renderer
{
public:
    static constexpr Pixel BLACK_PIXEL = 0x00'10'10'10;

    std::array<std::array<Pixel, PlaneU32::MAX_WIDTH>, 2> m_planeLine{};
    PlaneU32 m_screenARGB{384, 280, PlaneU32::MAX_SIZE};
    Pixel m_backdropColorARGB{0};
    PlaneU32 m_cursorPlaneARGB{PlaneU32::CURSOR_WIDTH, PlaneU32::CURSOR_HEIGHT, PlaneU32::CURSOR_SIZE};

    RendererSoftwareU32() {}
    virtual ~RendererSoftwareU32() noexcept {}

    std::pair<uint16_t, uint16_t> DrawLine(const uint8_t* lineA, const uint8_t* lineB) noexcept override;
    const Plane& RenderFrame() noexcept override;

    template<ImagePlane PLANE>
    uint16_t DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept;
    void DrawLineBackdrop() noexcept;
    void DrawCursor() noexcept;

    template<bool MIX, bool PLANE_ORDER> void OverlayMix() noexcept;

    /** \brief Handles the transparency of the current pixel for each plane.
     * \param pixel The ARGB pixel.
     */
    template<ImagePlane PLANE> void HandleTransparencyU32(uint32_t& pixel) noexcept
    {
        static constexpr uint32_t VISIBLE = 0xFF'00'00'00;

        const bool boolean = !bit<3>(m_transparencyControl[PLANE]);
        uint32_t color = pixel & 0x00'FF'FF'FF;
        color = clutColorKey(color | m_maskColorRgb[PLANE]);
        const bool colorKey = color == clutColorKey(m_transparentColorRgb[PLANE] | m_maskColorRgb[PLANE]); // TODO: don't compute if not CLUT.

        pixel |= VISIBLE;

        switch(bits<0, 2>(m_transparencyControl[PLANE]))
        {
        case 0b000: // Always/Never.
            if(boolean)
                pixel &= ~VISIBLE;
            break;

        case 0b001: // Color Key.
            if(colorKey == boolean)
                pixel &= ~VISIBLE;
            break;

        case 0b010: // Transparent Bit.
            // TODO: currently decodeRGB555 make the pixel visible if the bit is set.
            // TODO: disable if not RGB555.
            if(((pixel & VISIBLE) == VISIBLE) != boolean)
                pixel &= ~VISIBLE;
            break;

        case 0b011: // Matte Flag 0.
            if(m_matteFlags[0] == boolean)
                pixel &= ~VISIBLE;
            break;

        case 0b100: // Matte Flag 1.
            if(m_matteFlags[1] == boolean)
                pixel &= ~VISIBLE;
            break;

        case 0b101: // Matte Flag 0 or Color Key.
            if(m_matteFlags[0] == boolean || colorKey == boolean)
                pixel &= ~VISIBLE;
            break;

        case 0b110: // Matte Flag 1 or Color Key.
            if(m_matteFlags[1] == boolean || colorKey == boolean)
                pixel &= ~VISIBLE;
            break;

        default: // Reserved.
            break;
        }
    }
};

} // namespace Video

#endif // CDI_VIDEO_RENDERERSOFTWAREU32_HPP
