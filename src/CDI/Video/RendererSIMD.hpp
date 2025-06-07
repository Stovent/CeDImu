#ifndef CDI_VIDEO_RENDERERSIMD_HPP
#define CDI_VIDEO_RENDERERSIMD_HPP

#include "Renderer.hpp"
#include "SIMD.hpp"
#include "VideoSIMD.hpp"

#include <array>

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
};

} // namespace Video

#endif // CDI_VIDEO_RENDERERSIMD_HPP
