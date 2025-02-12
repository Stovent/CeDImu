#ifndef CDI_VIDEO_RENDERERSIMD_HPP
#define CDI_VIDEO_RENDERERSIMD_HPP

#include "Renderer.hpp"

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
    static constexpr size_t SIMD_LINE_WIDTH = (Plane::MAX_WIDTH * 4) + stdx::simd<uint8_t>::size() - ((Plane::MAX_WIDTH * 4) % stdx::simd<uint8_t>::size());
    std::array<std::array<uint8_t, SIMD_LINE_WIDTH>, 2> m_planeLine{};
    // std::array<std::array<uint32_t, Plane::MAX_WIDTH>, 2> m_planeLine{};

    RendererSIMD() {}

    std::pair<uint16_t, uint16_t> DrawLine(const uint8_t* lineA, const uint8_t* lineB) noexcept override;
    const Plane& RenderFrame() noexcept override;

    template<ImagePlane PLANE>
    uint16_t DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept;
    void DrawLineBackdrop() noexcept;
    void DrawCursor() noexcept;
    template<bool MIX, bool PLANE_ORDER> void OverlayMix() noexcept;
};

} // namespace Video

#endif // CDI_VIDEO_RENDERERSIMD_HPP
