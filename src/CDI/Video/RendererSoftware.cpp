#include "RendererSoftware.hpp"

#include "../common/panic.hpp"
#include "../common/utils.hpp"

#include <cstring>

namespace Video
{

/** \brief Converts the 4-bits backdrop color to RGB.
 * \param rgb The RGB destination buffer.
 * \param color The 4 bit color code.
 */
static constexpr void backdropColorToRGB(uint8_t* rgb, const uint8_t color) noexcept
{
    // Background plane has no transparency (Green book V.5.13).
    const uint8_t c = (color & 0x08) ? Renderer::PIXEL_FULL_INTENSITY : Renderer::PIXEL_HALF_INTENSITY;
    *rgb++ = (color & 0x04) ? c : 0; // Red.
    *rgb++ = (color & 0x02) ? c : 0; // Green.
    *rgb++ = (color & 0x01) ? c : 0; // Blue.
}

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
        Video::paste(m_screen.data(), m_screen.m_width, m_screen.m_height, m_cursorPlane.data(), m_cursorPlane.m_width, m_cursorPlane.m_height, m_cursorX >> 1, m_cursorY);
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
        const size_t length = m_plane[PLANE].m_width * m_plane[PLANE].m_bpp;
        memset(m_plane[PLANE](m_lineNumber), 0, length);
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
        return decodeBitmapLine(m_plane[PLANE](m_lineNumber), lineA, lineMain, m_plane[PLANE].m_width, clut, m_dyuvInitialValue[PLANE], m_codingMethod[PLANE]);

    case ImageType::RunLength:
        return decodeRunLengthLine(m_plane[PLANE](m_lineNumber), lineMain, m_plane[PLANE].m_width, clut, is4BPP);

    case ImageType::Mosaic:
        panic("Unsupported type Mosaic");
        return 0;
    }

    std::unreachable();
}

void RendererSoftware::DrawLineBackdrop() noexcept
{
    // The pixels of a line are all the same, so backdrop plane only contains the color of each line.
    backdropColorToRGB(m_backdropPlane(m_lineNumber), m_backdropColor);
}

void RendererSoftware::DrawCursor() noexcept
{
    // Technically speaking the cursor is drawn when the drawing line number is the cursor's one (because video
    // is outputted continuously line by line).
    // But for here maybe we don't care.

    // TODO: Should this use the same backdrops colors (V.5.12) ?
    const uint8_t A = bit<3>(m_cursorColor) ? PIXEL_FULL_INTENSITY : PIXEL_HALF_INTENSITY;
    const uint8_t R = bit<2>(m_cursorColor) ? PIXEL_FULL_INTENSITY : 0;
    const uint8_t G = bit<1>(m_cursorColor) ? PIXEL_FULL_INTENSITY : 0;
    const uint8_t B = bit<0>(m_cursorColor) ? PIXEL_FULL_INTENSITY : 0;

    uint8_t* pixels = m_cursorPlane(0); // Pixels are contiguous.
    for(int y = 0; y < m_cursorPlane.m_height; y++)
    {
        uint16_t mask = 1 << 15;
        for(uint8_t x = 0; x < m_cursorPlane.m_width; x++)
        {
            if(m_cursorPatterns[y] & mask)
            {
                *pixels++ = A;
                *pixels++ = R;
                *pixels++ = G;
                *pixels++ = B;
            }
            else
            {
                *pixels = 0;
                pixels += 4;
            }
            mask >>= 1;
        }
    }
}

/** \brief Apply the given Image Contribution Factor to the given color component (V.5.9). */
static constexpr uint8_t applyICF(const int color, const int icf) noexcept
{
    return static_cast<uint8_t>(((icf * (color - 16)) / 63) + 16);
}

/** \brief Apply mixing to the given color components after ICF (V.5.9.1). */
static constexpr uint8_t mix(const int a, const int b) noexcept
{
    return limu8(a + b - 16);
}

/** \brief Overlays or mix all the planes to the final screen.
 * \tparam MIX true to use mixing, false to use overlay.
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 *
 * TODO: for overlay, first check which pixel has to be shown, then compute ICF only for this pixel.
 * split in two different functions?
 */
template<bool MIX, bool PLANE_ORDER>
void RendererSoftware::OverlayMix() noexcept
{
    uint8_t* screen = m_screen(m_lineNumber);
    uint8_t* planeA = m_plane[A](m_lineNumber);
    uint8_t* planeB = m_plane[B](m_lineNumber);

    uint8_t* background = m_backdropPlane(m_lineNumber);
    const uint8_t rbg = background[0];
    const uint8_t gbg = background[1];
    const uint8_t bbg = background[2];

    for(uint16_t i = 0; i < m_plane[A].m_width; i++) // TODO: width[B].
    {
        HandleMatte<A>(i);
        HandleMatte<B>(i);

        HandleTransparency<A>(planeA);
        HandleTransparency<B>(planeB);

        const uint8_t aa = *planeA++;
        const uint8_t ra = applyICF(*planeA++, m_icf[A]);
        const uint8_t ga = applyICF(*planeA++, m_icf[A]);
        const uint8_t ba = applyICF(*planeA++, m_icf[A]);

        const uint8_t ab = *planeB++;
        const uint8_t rb = applyICF(*planeB++, m_icf[B]);
        const uint8_t gb = applyICF(*planeB++, m_icf[B]);
        const uint8_t bb = applyICF(*planeB++, m_icf[B]);

        uint8_t afp, rfp, gfp, bfp, abp, rbp, gbp, bbp;
        if constexpr(PLANE_ORDER) // Plane B in front.
        {
            afp = ab;
            rfp = rb;
            gfp = gb;
            bfp = bb;

            abp = aa;
            rbp = ra;
            gbp = ga;
            bbp = ba;
        }
        else // Plane A in front.
        {
            afp = aa;
            rfp = ra;
            gfp = ga;
            bfp = ba;

            abp = ab;
            rbp = rb;
            gbp = gb;
            bbp = bb;
        }

        uint8_t r, g, b;
        if constexpr(MIX)
        {
            if(abp == 0) // When mixing transparent pixels are black (V.5.9.1).
            {
                rbp = 16;
                gbp = 16;
                bbp = 16;
            }

            if(afp == 0)
            {
                rfp = 16;
                gfp = 16;
                bfp = 16;
            }

            r = mix(rbp, rfp);
            g = mix(gbp, gfp);
            b = mix(bbp, bfp);
        }
        else // Overlay.
        {
            // Plane transparency is either 0 or 255.
            if(afp == 0 && abp == 0) // Front and back plane transparent: only show background.
            {
                r = rbg;
                g = gbg;
                b = bbg;
            }
            else if(afp == 0) // Front plane transparent: show back plane.
            {
                r = rbp;
                g = gbp;
                b = bbp;
            }
            else // Front plane visible: only show front plane.
            {
                r = rfp;
                g = gfp;
                b = bfp;
            }
        }

        *screen++ = r;
        *screen++ = g;
        *screen++ = b;

        /*
        MCD212 figure 8-6
        For overlay/mix:
        - do the computation for a single pixel.
        - write it to the destination buffer with memcpy for mosaic and double resolution.
        */
    }
}

} // namespace Video
