#ifndef CDI_VIDEO_RENDERERSIMD_HPP
#define CDI_VIDEO_RENDERERSIMD_HPP

#include "Renderer.hpp"
#include "../common/VideoSIMD.hpp"

#include <array>

#include <experimental/simd>
namespace stdx = std::experimental; // remove when stable.
#if __cpp_lib_simd
#warning "SIMD is no longer experimental"
#endif

namespace Video
{

/** \brief CD-i video renderer implementation using std::simd.
 */
class RendererSIMD final : public Renderer
{
public:
    /** \brief Makes sure the dst buffer in aligned with std::simd::size(), as its written to in chunks of this size. */
    static constexpr size_t SIMD_LINE_WIDTH = SIMDAlign(PlaneSIMD::MAX_WIDTH);
    std::array<std::array<PixelU32, SIMD_LINE_WIDTH>, 2> m_planeLine{};
    PlaneSIMD m_screenARGB{384, 280, SIMDAlign(PlaneSIMD::ARGB_MAX_SIZE)};
    PixelU32 m_backdropColorARGB{0};
    PlaneSIMD m_cursorPlaneARGB{PlaneSIMD::CURSOR_WIDTH, PlaneSIMD::CURSOR_HEIGHT, PlaneSIMD::CURSOR_ARGB_SIZE};

    std::array<std::array<uint8_t, SIMD_LINE_WIDTH>, 2> m_icfLine{};
    // std::array<uint8_t, 2> m_currentICF{};
    // std::array<std::array<uint8_t, SIMD_LINE_WIDTH>, 2> m_matteFlagLine{};
    // uint32_t m_currentMatteCommand{0};

    RendererSIMD() {}
    virtual ~RendererSIMD() noexcept {}

    std::pair<uint16_t, uint16_t> DrawLine(const uint8_t* lineA, const uint8_t* lineB) noexcept override;
    const Plane& RenderFrame() noexcept override;

    template<ImagePlane PLANE>
    uint16_t DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept;
    void DrawLineBackdrop() noexcept;
    void DrawCursor() noexcept;

    template<bool MIX, bool PLANE_ORDER> void OverlayMix() noexcept;
    template<bool PLANE_ORDER> void ApplyICFMixSIMDShift() noexcept;
    template<bool PLANE_ORDER> void ApplyICFMixSIMDCast() noexcept;
    template<bool PLANE_ORDER> void ApplyICFOverlaySIMD() noexcept;

    void ResetMatteSIMD() noexcept;

    /** \brief Handles the transparency of the current pixel for each plane.
     * \param pixel The ARGB pixel.
     *
     * TODO: this may be SIMDable.
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
