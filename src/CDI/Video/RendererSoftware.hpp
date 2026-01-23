#ifndef CDI_VIDEO_RENDERERSOFTWARE_HPP
#define CDI_VIDEO_RENDERERSOFTWARE_HPP

#include "Renderer.hpp"

namespace Video
{

/** \brief Generic CD-i video renderer implementation.
 */
class RendererSoftware final : public Renderer
{
public:
    RendererSoftware() {}
    virtual ~RendererSoftware() noexcept {}

    virtual std::pair<uint16_t, uint16_t> DrawLineImpl(const uint8_t* lineA, const uint8_t* lineB) noexcept override;
    virtual void DrawCursor() noexcept override;

    template<ImagePlane PLANE>
    uint16_t DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept;
    template<bool MIX, bool PLANE_ORDER> void OverlayMix() noexcept;

    void HandleMatteAndTransparency(uint16_t lineNumber) noexcept;
    template<TransparentIf TRANSPARENCY_A, bool FLAG_A>
    void HandleMatteAndTransparencyDispatchB(uint16_t lineNumber) noexcept;
    template<TransparentIf TRANSPARENCY_A, bool FLAG_A, TransparentIf TRANSPARENCY_B, bool FLAG_B>
    void HandleMatteAndTransparencyLoop(uint16_t lineNumber) noexcept;
    template<ImagePlane PLANE, TransparentIf TRANSPARENT, bool BOOL_FLAG>
    constexpr void HandleTransparency(Pixel& pixel) noexcept;

    // Image Contribution Factor.
    std::array<std::array<uint8_t, Plane::MAX_WIDTH>, 2> m_icfLine{}; /**< ICF for the whole line. */

    // Matte (Region of the MCD212).
    std::array<uint8_t, 2> m_nextMatte{};
    template<ImagePlane PLANE> void HandleMatte(uint16_t pos) noexcept;
};

} // namespace Video

#endif // CDI_VIDEO_RENDERERSOFTWARE_HPP
