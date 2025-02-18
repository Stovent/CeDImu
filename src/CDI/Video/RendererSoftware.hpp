#ifndef CDI_VIDEO_RENDERERSOFTWARE_HPP
#define CDI_VIDEO_RENDERERSOFTWARE_HPP

#include "Renderer.hpp"
#include "../common/Video.hpp"

namespace Video
{

/** \brief CD-i video renderer implementation using only C++ code.
 *
 * TODO: use uint32_t instead of uint8_t?
 */
class RendererSoftware final : public Renderer
{
public:
    RendererSoftware() {}
    virtual ~RendererSoftware() noexcept {}

    std::pair<uint16_t, uint16_t> DrawLine(const uint8_t* lineA, const uint8_t* lineB) noexcept override;
    const Plane& RenderFrame() noexcept override;

    template<ImagePlane PLANE>
    uint16_t DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept;
    void DrawLineBackdrop() noexcept;
    void DrawCursor() noexcept;
    template<bool MIX, bool PLANE_ORDER> void OverlayMix() noexcept;
};

} // namespace Video

#endif // CDI_VIDEO_RENDERERSOFTWARE_HPP
