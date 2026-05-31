/** \file RendererWGPU.cpp
 * \brief Implementation of RendererWGPU class.
 *
 * The idea is that GPU computing has an overhead before doing the computations, so we can't do line by line.
 * Instead, on each line the matte flags are calculated, and the line input data with its metadata is cached in a buffer.
 * When the frame is complete, send everything to the GPU and actually generate the frame.
 *
 * TODO: experiment sending each line data asynchrosously.
 * TODO: for run length: decode it on the GPU anyway, with only the thread 0 of the line doing it?
 *
 * For now because run-length cannot be done on the GPU, the first plane decoding is done on the CPU,
 * then matte/transparency and overlay/mixing is done on the GPU.
 */

#include "RendererWGPU.hpp"
#include "VideoDecoders.hpp"

#include "../common/panic.hpp"
#include "../common/utils.hpp"

#include <cstring>

namespace Video
{

/** \brief Draws the next line to draw.
 * \param lineA Line A data.
 * \param lineB Line B data.
 * \param lineNumber The line number to draw (starting at 0).
 * \return The number of bytes read from memory for each plane `<plane A, plane B>`.
 */
std::pair<uint16_t, uint16_t> RendererWGPU::DrawLineImpl(const uint8_t* lineA, const uint8_t* lineB) noexcept
{
    // Save the width and height of the plane for the metadata.
    if(m_lineNumber == 0) [[unlikely]]
    {
        m_transparencyFrame[A].m_height = m_transparencyFrame[B].m_height =
        m_matteCommands.m_height = m_matteNumbers.m_height =
        m_initialICF[A].m_height = m_initialICF[B].m_height
        = m_screen.m_height;
    }

    uint16_t bytesA = DrawLinePlane<A>(lineA, nullptr); // nullptr because plane A can't decode RGB555.
    const uint16_t bytesB = DrawLinePlane<B>(lineB, lineA);
    if(m_codingMethod[B] == ImageCodingMethod::RGB555)
        bytesA = bytesB;

    // Store the display parameters for this line.
    *m_transparencyFrame[A].GetLinePointer(m_lineNumber) = bits<0, 3>(m_transparencyControl[A]);
    *m_transparencyFrame[B].GetLinePointer(m_lineNumber) = bits<0, 3>(m_transparencyControl[B]);
    *m_maskColorFrame[A].GetLinePointer(m_lineNumber) = m_maskColorRgb[A];
    *m_maskColorFrame[B].GetLinePointer(m_lineNumber) = m_maskColorRgb[B];
    *m_transparentColorFrame[A].GetLinePointer(m_lineNumber) = m_transparentColorRgb[A];
    *m_transparentColorFrame[B].GetLinePointer(m_lineNumber) = m_transparentColorRgb[B];
    memcpy(m_matteCommands.GetLinePointer(m_lineNumber), m_matteControl.data(), MATTE_NUM * sizeof(uint32_t));
    *m_matteNumbers.GetLinePointer(m_lineNumber) = m_matteNumber;
    *m_initialICF[A].GetLinePointer(m_lineNumber) = m_icf[A];
    *m_initialICF[B].GetLinePointer(m_lineNumber) = m_icf[B];

    return std::make_pair(bytesA, bytesB);
}

/** \brief Draws the line of the given plane.
 * \param lineMain Line that will be decoded.
 * \param lineA Line A data if RGB555.
 * \return The number of bytes read from memory.
 *
 * lineA is only used when the decoding method is RGB555.
 */
template<ImagePlane PLANE>
uint16_t RendererWGPU::DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept
{
    if(m_codingMethod[PLANE] == ImageCodingMethod::OFF)
    {
        std::fill_n(m_plane[PLANE].GetLinePointer(m_lineNumber), m_plane[PLANE].m_width, Pixel{0});
        return 0;
    }

    const uint32_t* clut;
    if constexpr(PLANE == A)
        clut = m_codingMethod[A] == ICM(CLUT77) && m_clutSelectHigh ? &m_clut[128] : m_clut.data();
    else
        clut = &m_clut[128];

    const ImageCodingMethod icm = m_codingMethod[PLANE];

    switch(m_imageType[PLANE])
    {
    case ImageType::Normal:
        if(icm == ImageCodingMethod::CLUT4)
            if(Is360Pixels())
                return decodeBitmapLine<720>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineA, lineMain, clut, m_dyuvInitialValue[PLANE], icm);
            else
                return decodeBitmapLine<768>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineA, lineMain, clut, m_dyuvInitialValue[PLANE], icm);
        else
            if(Is360Pixels())
                return decodeBitmapLine<360>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineA, lineMain, clut, m_dyuvInitialValue[PLANE], icm);
            else
                return decodeBitmapLine<384>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineA, lineMain, clut, m_dyuvInitialValue[PLANE], icm);

    case ImageType::RunLength:
        if(m_bps[PLANE] == BitsPerPixel::Double4) // RL3
            if(Is360Pixels())
                return decodeRunLengthLine<720, true>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineMain, clut);
            else
                return decodeRunLengthLine<768, true>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineMain, clut);
        else if(m_bps[PLANE] == BitsPerPixel::High8) // RL7 high
            if(Is360Pixels())
                return decodeRunLengthLine<720, false>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineMain, clut);
            else
                return decodeRunLengthLine<768, false>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineMain, clut);
        else
            if(Is360Pixels())
                return decodeRunLengthLine<360, false>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineMain, clut);
            else
                return decodeRunLengthLine<384, false>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineMain, clut);

    case ImageType::Mosaic:
        panic("Unsupported type Mosaic");
        return 0;
    }

    std::unreachable();
}

void RendererWGPU::DrawCursor() noexcept
{
    // Technically speaking the cursor is drawn when the drawing line number is the cursor's one (because video
    // is outputted continuously line by line). But for here maybe we don't care.
    const Pixel color = GetCursorColor();

    Plane::iterator it = m_cursorPlane.begin();
    for(size_t y = 0; y < m_cursorPlane.m_height; ++y)
    {
        for(int x = static_cast<int>(m_cursorPlane.m_width) - 1; x >= 0; --x)
        {
            const uint16_t mask = (1 << x);
            if(m_cursorPatterns[y] & mask)
                *it = color;
            else
                *it = BLACK_PIXEL;
            ++it;
        }
    }
}

void RendererWGPU::RenderFrameImpl() noexcept
{
    const WgpuRendererFrame frame = {
        .width = static_cast<uint32_t>(m_screen.m_width),
        .height = static_cast<uint32_t>(m_screen.m_height),
        .mix = m_mix, // TODO: those are per line too.
        .front_plane_b = m_planeOrder,
    };

    renderer_wgpu_render(m_wgpuRenderer.get(), &frame, &m_buffers);
}

} // namespace Video
