/** \file RendererSoftware.cpp
 * \brief RendererSoftware implementation file.
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
 * \param lineNumber The line number to draw (starting at 0).
 * \return The number of bytes read from memory for each plane `<plane A, plane B>`.
 */
std::pair<uint16_t, uint16_t> RendererSoftware::DrawLine(const uint8_t* lineA, const uint8_t* lineB, const uint16_t lineNumber) noexcept
{
    m_lineNumber = lineNumber;
    if(m_lineNumber == 0)
    {
        uint16_t width = getDisplayWidth(m_displayFormat);
        uint16_t height = GetDisplayHeight();

        m_screen.m_width = m_plane[A].m_width = m_plane[B].m_width = width * 2;
        m_screen.m_height = m_plane[A].m_height = m_plane[B].m_height = height;
    }

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

    return std::make_pair(bytesA, bytesB);
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

void RendererSoftware::DrawCursor() noexcept
{
    // Technically speaking the cursor is drawn when the drawing line number is the cursor's one (because video
    // is outputted continuously line by line).
    // But for here maybe we don't care.
    Pixel color = backdropCursorColorToPixel(m_cursorColor);
    if(!m_cursorIsOn)
    {
        if(m_cursorBlinkType) // Complement.
            color = color.Complement();
        else
            color = BLACK_PIXEL;
    }

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

    for(uint16_t i = 0; i < m_plane[A].m_width; i++) // Both planes always have the same width.
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
