#ifndef CDI_VIDEO_RENDERERSIMD_HPP
#define CDI_VIDEO_RENDERERSIMD_HPP

#include "Renderer.hpp"

namespace Video
{

/** \brief CD-i video renderer implementation using std::simd.
 */
class RendererSIMD final : public Renderer
{
public:
    RendererSIMD() {}
    virtual ~RendererSIMD() noexcept {}

    std::pair<uint16_t, uint16_t> DrawLine(const uint8_t* lineA, const uint8_t* lineB, uint16_t lineNumber) noexcept override;
    void DrawCursor() noexcept override;

    template<ImagePlane PLANE>
    uint16_t DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept;
    template<bool MIX, bool PLANE_ORDER> void OverlayMix() noexcept;
    template<bool MIX, bool PLANE_ORDER, size_t WIDTH_REMINDER> void HandleOverlayMixSIMD() noexcept;

    void HandleTransparencyPlaneASIMD() noexcept;
    void HandleTransparencyPlaneBSIMD() noexcept;
    template<Renderer::ImagePlane PLANE, Renderer::TransparentIf TRANSPARENT, bool BOOL_FLAG>
    void HandleTransparencyLoopSIMD() noexcept;

    template<bool TWO_MATTES>
    void HandleMatteSIMD() noexcept;
    template<bool TWO_MATTES>
    bool ExecuteMatteCommand(uint32_t command, bool mf) noexcept;

    std::array<std::array<bool, Plane::MAX_WIDTH>, 2> m_matteFlagsLine{}; /**< Matte flags for the whole line. */
};

} // namespace Video

#endif // CDI_VIDEO_RENDERERSIMD_HPP
