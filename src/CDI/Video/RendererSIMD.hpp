#ifndef CDI_VIDEO_RENDERERSIMD_HPP
#define CDI_VIDEO_RENDERERSIMD_HPP

#include "Renderer.hpp"
#include "VideoSIMD.hpp"

#include <array>

#include <experimental/simd>
namespace stdx = std::experimental; // remove when stable.
#if __has_include(<simd>)
#warning "SIMD is no longer experimental"
#endif

namespace Video
{

/** \brief CD-i video renderer implementation using std::simd.
 *
 * TODO: allow VDSC Viewer to show the raw planes with transparency.
 * TODO: change the algorithms to adapt to the remaining size of pixels to process.
 * Basically the video resolutions are always multiples of 8, and sometimes multiples of 16.
 * There may be something interesting to do with this.
 * Either:
 * - use a Fixed-width SIMD based on multiple of 8 or 16.
 * - loop with the max width SIMD, until the last loop where the non-multiple remaining pixels are processed.
 */
class RendererSIMD final : public Renderer
{
public:
    /** \brief Makes sure the dst buffer in aligned with std::simd::size(), as its written to in chunks of this size. */
    static constexpr size_t SIMD_LINE_WIDTH = SIMDAlign(Plane::MAX_WIDTH);
    std::array<std::array<PixelU32, SIMD_LINE_WIDTH>, 2> m_planeLine{};

    std::array<std::array<uint8_t, SIMD_LINE_WIDTH>, 2> m_icfLine{};

    RendererSIMD() {}
    virtual ~RendererSIMD() noexcept {}

    std::pair<uint16_t, uint16_t> DrawLine(const uint8_t* lineA, const uint8_t* lineB) noexcept override;
    const Plane& RenderFrame() noexcept override;

    template<ImagePlane PLANE>
    uint16_t DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept;
    void DrawLineBackdrop() noexcept;
    void DrawCursor() noexcept;

    template<bool MIX, bool PLANE_ORDER> void OverlayMix() noexcept;
    template<bool MIX, bool PLANE_ORDER> void HandleOverlayMixSIMD() noexcept;

    void ResetMatteSIMD() noexcept;

    /** \brief Handles the transparency of the current pixel for each plane.
     * \param pixel The ARGB pixel.
     *
     * TODO: this may be SIMDable.
     * It seems that transparency control remains the same for the entire line.
     * Can we use this to remove the redundent switch on each loop?
     */
    template<ImagePlane PLANE> void HandleTransparencySIMD(uint32_t& pixel) noexcept
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

#endif // CDI_VIDEO_RENDERERSIMD_HPP
