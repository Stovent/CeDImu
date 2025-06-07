/** \file RendererSoftware.cpp
 * \brief RendererSoftware implementation file.
 *
 * TODO: The individual planes does not store transparency anymore like the previous renderer.
 * This should be restored.
 */

#include "RendererSoftware.hpp"
#include "VideoDecoders.hpp"

#include "../common/panic.hpp"
#include "../common/utils.hpp"

#include <cstring>

namespace Video
{

/** \brief Draws the next line to draw.
 * \param lineA Line A data.
 * \param lineB Line B data.
 * \return The number of bytes read from memory for each plane `<plane A, plane B>`.
 */
std::pair<uint16_t, uint16_t> RendererSoftware::DrawLine(const uint8_t* lineA, const uint8_t* lineB) noexcept
{
    ResetMatte();

    const uint16_t bytesA = DrawLinePlane<A>(lineA, nullptr); // nullptr because plane A can't decode RGB555.
    const uint16_t bytesB = DrawLinePlane<B>(lineB, lineA);

    DrawLineBackdrop();

    if(m_mix)
        if(m_planeOrder)
            OverlayMix<true, true>();
        else
            OverlayMix<true, false>();
    else
        if(m_planeOrder)
            OverlayMix<false, true>();
        else
            OverlayMix<false, false>();

    m_lineNumber++;
    return std::make_pair(bytesA, bytesB);
}

/** \brief To be called when the whole frame is drawn.
 * \return The final screen.
 *
 * This function renders the cursor and pastes it on the screen.
 * It also resets some members to prepare for the next frame.
 */
const Plane& RendererSoftware::RenderFrame() noexcept
{
    // Should this be inside DrawLine() ?
    if(m_cursorEnabled)
    {
        DrawCursor();
        // TODO: double resolution.
        Video::paste(m_screen.data(), m_screen.m_width, m_screen.m_height,
                     m_cursorPlane.data(), m_cursorPlane.m_width, m_cursorPlane.m_height,
                     m_cursorX >> 1, m_cursorY);
    }

    m_lineNumber = 0;
    return m_screen;
}

/** \brief Draws the line of the given plane.
 * \param lineMain Line that will be decoded.
 * \param lineA Line A data if RGB555.
 * \return The number of bytes read from memory.
 *
 * lineA is only used when the decoding method is RGB555.
 */
template<Renderer::ImagePlane PLANE>
uint16_t RendererSoftware::DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept
{
    if(m_codingMethod[PLANE] == ImageCodingMethod::OFF)
    {
        std::fill_n(m_plane[PLANE].begin(), m_plane[PLANE].m_width, Pixel{0});
        return 0;
    }

    const bool is4BPP = false; // TODO.

    const uint32_t* clut;
    if constexpr(PLANE == A)
        clut = m_codingMethod[A] == ICM(CLUT77) && m_clutSelectHigh ? &m_clut[128] : m_clut.data();
    else
        clut = &m_clut[128];

    switch(m_imageType[PLANE])
    {
    case ImageType::Normal:
        return decodeBitmapLine(m_plane[PLANE].GetLinePointer(m_lineNumber), lineA, lineMain, m_plane[PLANE].m_width, clut, m_dyuvInitialValue[PLANE], m_codingMethod[PLANE]);

    case ImageType::RunLength:
        return decodeRunLengthLine(m_plane[PLANE].GetLinePointer(m_lineNumber), lineMain, m_plane[PLANE].m_width, clut, is4BPP);

    case ImageType::Mosaic:
        panic("Unsupported type Mosaic");
        return 0;
    }

    std::unreachable();
}

void RendererSoftware::DrawLineBackdrop() noexcept
{
    // The pixels of a line are all the same, so backdrop plane only contains the color of each line.
    *m_backdropPlane.GetLinePointer(m_lineNumber) = backdropCursorColorToPixel(m_backdropColor);
}

void RendererSoftware::DrawCursor() noexcept
{
    // Technically speaking the cursor is drawn when the drawing line number is the cursor's one (because video
    // is outputted continuously line by line).
    // But for here maybe we don't care.
    const Pixel color = backdropCursorColorToPixel(m_cursorColor);

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

/** \brief Apply the given Image Contribution Factor to the given color component (V.5.9). */
static constexpr uint8_t applyICFComponent(const int color, const int icf) noexcept
{
    return static_cast<uint8_t>(((icf * (color - 16)) / 63) + 16);
}

/** \brief Apply the given Image Contribution Factor to the given color component (V.5.9). */
static constexpr void applyICF(Pixel& pixel, const int icf) noexcept
{
    pixel.r = applyICFComponent(pixel.r, icf);
    pixel.g = applyICFComponent(pixel.g, icf);
    pixel.b = applyICFComponent(pixel.b, icf);
}

/** \brief Apply mixing to the given color components after ICF (V.5.9.1). */
static constexpr void mix(Pixel& dst, const Pixel a, const Pixel b) noexcept
{
    dst.r = limu8(a.r + b.r - 16);
    dst.g = limu8(a.g + b.g - 16);
    dst.b = limu8(a.b + b.b - 16);
}

/** \brief Overlays or mix all the planes to the final screen.
 * \tparam MIX true to use mixing, false to use overlay.
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 */
template<bool MIX, bool PLANE_ORDER>
void RendererSoftware::OverlayMix() noexcept
{
    Pixel* screen = m_screen.GetLinePointer(m_lineNumber);
    Pixel* planeA = m_plane[A].GetLinePointer(m_lineNumber);
    Pixel* planeB = m_plane[B].GetLinePointer(m_lineNumber);
    const Pixel backdrop = *m_backdropPlane.GetLinePointer(m_lineNumber);

    HandleMatteAndTransparency(m_lineNumber);

    for(uint16_t i = 0; i < m_plane[A].m_width; i++) // TODO: width[B].
    {
        Pixel a = *planeA++;
        Pixel b = *planeB++;

        applyICF(a, m_icfLine[A][i]);
        applyICF(b, m_icfLine[B][i]);

        Pixel fp, bp;
        if constexpr(PLANE_ORDER) // Plane B in front.
        {
            fp = b;
            bp = a;
        }
        else // Plane A in front.
        {
            fp = a;
            bp = b;
        }

        if constexpr(MIX)
        {
            if(fp.a == 0) // When mixing transparent pixels are black (V.5.9.1).
            {
                fp = BLACK_PIXEL;
            }

            if(bp.a == 0)
            {
                bp = BLACK_PIXEL;
            }

            mix(*screen++, fp, bp);
        }
        else // Overlay.
        {
            Pixel pixel;
            // Plane transparency is either 0 or 255.
            if(fp.a == 0 && bp.a == 0) // Front and back plane transparent: only show background.
            {
                pixel = backdrop;
            }
            else if(fp.a == 0) // Front plane transparent: show back plane.
            {
                pixel = bp;
            }
            else // Front plane visible: only show front plane.
            {
                pixel = fp;
            }
            *screen++ = pixel;
        }
    }
}

} // namespace Video
