#ifndef CDI_VIDEO_RENDERERSIMD_HPP
#define CDI_VIDEO_RENDERERSIMD_HPP

#include "Renderer.hpp"
#include "SIMD.hpp"
#include "VideoDecodersSIMD.hpp"

#include <array>

namespace Video
{

/** \brief CD-i video renderer implementation using std::simd.
 */
class RendererSIMD final : public Renderer
{
public:
    RendererSIMD() {}
    virtual ~RendererSIMD() noexcept {}

    std::pair<uint16_t, uint16_t> DrawLine(const uint8_t* lineA, const uint8_t* lineB) noexcept override;
    void DrawCursor() noexcept override;

    template<ImagePlane PLANE>
    uint16_t DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept;
    template<bool MIX, bool PLANE_ORDER> void OverlayMix() noexcept;
    template<bool MIX, bool PLANE_ORDER, size_t WIDTH_REMINDER> void HandleOverlayMixSIMD() noexcept;
};

} // namespace Video

#endif // CDI_VIDEO_RENDERERSIMD_HPP
