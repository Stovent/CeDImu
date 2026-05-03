#ifndef CDI_VIDEO_RENDERERWGPU_HPP
#define CDI_VIDEO_RENDERERWGPU_HPP

#include "Renderer.hpp"
#include "RendererWGPU_ffi.hpp"

#include <memory>

struct WgpuRendererContextDeleter {
    void operator()(WgpuRendererContext* p) const {
        renderer_wgpu_delete(p);
    }
};

namespace Video
{

/** \brief CD-i video renderer implementation using the rust crate wgpu.
 * According to RenderDoc the performances are heavily blocked by the transfert of the buffers to and from the GPU.
 * Possible optimizations:
 * - on each line, decode the two planes and send them to the GPU but return before the transfert is complete (async).
 * - once all lines are transfered, dispatch compute AND async read back, while running the CPU again.
 * - Maybe combine all at once? send data + compute + read back as a single async dispatch while the emulation continues?
 */
class RendererWGPU final : public Renderer
{
public:
    RendererWGPU() : m_wgpuRenderer{renderer_wgpu_new()},
        m_buffers{
            .screen = m_screen[0].AsU8Pointer(),
            .plane_a = m_plane[A][0].AsU8Pointer(),
            .plane_b = m_plane[B][0].AsU8Pointer(),
            .background = m_backdropPlane[0].AsU8Pointer(),
            .transparency_a = m_transparencyFrame[A].data(),
            .transparency_b = m_transparencyFrame[B].data(),
            .mask_color_a = m_maskColorFrame[A][0].AsU8Pointer(),
            .mask_color_b = m_maskColorFrame[B][0].AsU8Pointer(),
            .transparent_color_a = m_transparentColorFrame[A][0].AsU8Pointer(),
            .transparent_color_b = m_transparentColorFrame[B][0].AsU8Pointer(),
            .matte_commands = reinterpret_cast<const uint8_t*>(m_matteCommands.data()),
            .matte_number = m_matteNumbers.data(),
            .initial_icf_a = m_initialICF[A].data(),
            .initial_icf_b = m_initialICF[B].data(),
        }
    {}
    virtual ~RendererWGPU() noexcept {}

    virtual std::pair<uint16_t, uint16_t> DrawLineImpl(const uint8_t* lineA, const uint8_t* lineB) noexcept override;
    virtual void DrawCursor() noexcept override;
    virtual void RenderFrameImpl() noexcept override;

    template<ImagePlane PLANE>
    uint16_t DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept;
    template<bool MIX, bool PLANE_ORDER> void OverlayMix() noexcept;

    std::array<PlaneU8, 2> m_transparencyFrame{PlaneU8{1, 0, Plane::MAX_HEIGHT}, PlaneU8{1, 0, Plane::MAX_HEIGHT}}; /**< Transparency mechanism for each line. */
    std::array<Plane, 2> m_maskColorFrame{Plane{1, 0, Plane::MAX_HEIGHT}, Plane{1, 0, Plane::MAX_HEIGHT}};
    std::array<Plane, 2> m_transparentColorFrame{Plane{1, 0, Plane::MAX_HEIGHT}, Plane{1, 0, Plane::MAX_HEIGHT}};
    PlaneU32 m_matteCommands{MATTE_NUM, 0, Plane::MAX_HEIGHT * MATTE_NUM}; /**< Matte commands for each line. */
    PlaneU8 m_matteNumbers{1, 0, Plane::MAX_HEIGHT}; /**< Number of matte flags for each line. */
    std::array<PlaneU8, 2> m_initialICF{PlaneU8{1, 0, Plane::MAX_HEIGHT}, PlaneU8{1, 0, Plane::MAX_HEIGHT}}; /**< Initial value for the line of ICF. */

    std::unique_ptr<WgpuRendererContext, WgpuRendererContextDeleter> m_wgpuRenderer;
    WgpuRendererBuffersFfi m_buffers; // Must be the last one, so that the previous buffers are valid when initializing this.
};

} // namespace Video

#endif // CDI_VIDEO_RENDERERWGPU_HPP
