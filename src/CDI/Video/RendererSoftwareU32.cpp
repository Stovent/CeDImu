#include "RendererSoftwareU32.hpp"

#include "../common/panic.hpp"
#include "../common/utils.hpp"

#include <execution>
#include <cstring>

namespace Video
{

/** \brief Converts the 4-bits backdrop color to ARGB.
 * \param color The 4 bit color code.
 * \returns The ARGB color.
 */
static constexpr uint32_t backdropCursorColorToARGB(const uint8_t color) noexcept
{
    // Background plane has no transparency (Green book V.5.13).
    const uint32_t c = (bit<3>(color)) ? Renderer::PIXEL_FULL_INTENSITY : Renderer::PIXEL_HALF_INTENSITY;
    uint32_t argb = 0xFF'00'00'00; // Set transparency for cursor plane.
    if(bit<2>(color)) argb |= c << 16; // Red.
    if(bit<1>(color)) argb |= c << 8; // Green.
    if(bit<0>(color)) argb |= c; // Blue.
    return argb;
}

/** \brief Draws the next line to draw.
 * \param lineA Line A data.
 * \param lineB Line B data.
 * \return The number of bytes read from memory for each plane `<plane A, plane B>`.
 */
std::pair<uint16_t, uint16_t> RendererSoftwareU32::DrawLine(const uint8_t* lineA, const uint8_t* lineB) noexcept
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
const Plane& RendererSoftwareU32::RenderFrame() noexcept
{
    // Should this be inside DrawLine() ?
    if(m_cursorEnabled)
    {
        DrawCursor();
        // TODO: double resolution
        Video::paste(m_screenARGB.data(), m_screen.m_width, m_screen.m_height, m_cursorPlaneARGB.data(), m_cursorPlaneARGB.m_width, m_cursorPlaneARGB.m_height, m_cursorX >> 1, m_cursorY);
    }

    // TODO: this should be on the GUI side.
    Plane::iterator dstit = m_screen.begin();
    PlaneU32::const_iterator it = m_screenARGB.cbegin();
    for(size_t i = 0; i < m_screenARGB.PixelCount(); ++i, ++it, dstit += 3)
    {
        const Pixel pixel = *it;
        dstit[0] = pixel.r;
        dstit[1] = pixel.g;
        dstit[2] = pixel.b;
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
uint16_t RendererSoftwareU32::DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept
{
    if(m_codingMethod[PLANE] == ImageCodingMethod::OFF)
    {
        std::fill_n(std::execution::par_unseq, m_planeLine[PLANE].begin(), m_plane[PLANE].m_width, 0);
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
        return decodeBitmapLineU32(m_planeLine[PLANE].data(), lineA, lineMain, m_plane[PLANE].m_width, clut, m_dyuvInitialValue[PLANE], m_codingMethod[PLANE]);

    case ImageType::RunLength:
        return decodeRunLengthLineU32(m_planeLine[PLANE].data(), lineMain, m_plane[PLANE].m_width, clut, is4BPP);

    case ImageType::Mosaic:
        panic("Unsupported type Mosaic");
        return 0;
    }

    std::unreachable();
}

void RendererSoftwareU32::DrawLineBackdrop() noexcept
{
    // The pixels of a line are all the same, so backdrop plane only contains the color of each line.
    m_backdropColorARGB = backdropCursorColorToARGB(m_backdropColor);
}

void RendererSoftwareU32::DrawCursor() noexcept
{
    // Technically speaking the cursor is drawn when the drawing line number is the cursor's one (because video
    // is outputted continuously line by line).
    // But for here maybe we don't care.
    const Pixel color = backdropCursorColorToARGB(m_cursorColor);
    const Pixel black{0};

    PlaneU32::iterator it = m_cursorPlaneARGB.begin();
    for(int y = 0; y < m_cursorPlaneARGB.m_height; ++y)
    {
        for(int x = m_cursorPlaneARGB.m_width - 1; x >= 0; --x)
        {
            const uint16_t mask = (1 << x);
            if(m_cursorPatterns[y] & mask)
                *it = color;
            else
                *it = black;
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
void RendererSoftwareU32::OverlayMix() noexcept
{
    Pixel* screen = m_screenARGB.GetLinePointer(m_lineNumber);
    Pixel* planeA = m_planeLine[A].data();
    Pixel* planeB = m_planeLine[B].data();

    for(uint16_t i = 0; i < m_plane[A].m_width; i++) // TODO: width[B].
    {
        HandleMatte<A>(i);
        HandleMatte<B>(i);

        Pixel pa = *planeA++;
        Pixel pb = *planeB++;

        HandleTransparencyU32<A>(pa);
        HandleTransparencyU32<B>(pb);

        applyICF(pa, m_icf[A]);
        applyICF(pb, m_icf[B]);

        Pixel fp, bp;
        if constexpr(PLANE_ORDER) // Plane B in front.
        {
            fp = pb;
            bp = pa;
        }
        else // Plane A in front.
        {
            fp = pa;
            bp = pb;
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
                pixel = m_backdropColorARGB;
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
