#include "RendererSoftwareU32.hpp"

#include "../common/panic.hpp"
#include "../common/utils.hpp"

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
        Video::paste(m_screen.data(), m_screen.m_width, m_screen.m_height, m_cursorPlane.data(), m_cursorPlane.m_width, m_cursorPlane.m_height, m_cursorX, m_cursorY);
    }

    // TODO: this should be on the GUI side.
    Plane::iterator dstit = m_screen.begin();
    PlaneU32::const_iterator it = m_screenARGB.cbegin();
    for(size_t i = 0; i < m_screenARGB.PixelCount(); ++i, ++it, dstit += 3)
    {
        const Pixel pixel = *it;
        dstit[0] = pixel >> 16;
        dstit[1] = pixel >> 8;
        dstit[2] = pixel;
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
        const size_t length = m_plane[PLANE].m_width * m_plane[PLANE].m_bpp;
        memset(m_planeLine[PLANE].data(), 0, length);
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

    int pattern = 0;
    for(PlaneU32::iterator it = m_cursorPlaneARGB.begin(); it < m_cursorPlaneARGB.end();)
    {
        for(int x = 0; x < m_cursorPlaneARGB.m_width; ++x)
        {
            const uint16_t mask = (1 << x);
            if(m_cursorPatterns[pattern] & mask)
                *it = color;
            else
                *it = black;
            ++it;
        }

        ++pattern;
    }
}

static constexpr uint8_t intByteMult(uint32_t color1, uint32_t color2) noexcept {
    return static_cast<uint8_t>(((color1 * (color2 | color2 << 8)) + 0x8080) >> 16);
}

/** \brief Apply the given Image Contribution Factor to the given color component (V.5.9). */
static constexpr uint32_t applyICFComponent(const int color, const int icf) noexcept
{
    return static_cast<uint8_t>(((icf * (color - 16)) / 63) + 16);
}

/** \brief Apply the given Image Contribution Factor to the given color component (V.5.9). */
static constexpr Pixel applyICF(const Pixel pixel, const int icf) noexcept
{
    // return intByteMult((icf >> 4) + (icf << 2), pixel - 16) + 16;

    uint8_t r = pixel >> 16;
    uint8_t g = pixel >> 8;
    uint8_t b = pixel;

    return (pixel & 0xFF'00'00'00) | applyICFComponent(r, icf) << 16 | applyICFComponent(g, icf) << 8 | applyICFComponent(b, icf);
}

/** \brief Apply mixing to the given color components after ICF (V.5.9.1). */
static constexpr Pixel mix(const Pixel a, const Pixel b) noexcept
{
    // return limu8(a + b - 16);

    uint8_t ra = a >> 16;
    uint8_t ga = a >> 8;
    uint8_t ba = a;

    uint8_t rb = b >> 16;
    uint8_t gb = b >> 8;
    uint8_t bb = b;

    return limu8(ra + rb - 16) << 16 | limu8(ga + gb - 16) << 8 | limu8(ba + bb - 16);
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

        HandleTransparencyU32<A>(*planeA);
        HandleTransparencyU32<B>(*planeB);

        Pixel pa = applyICF(*planeA++, m_icf[A]);
        Pixel pb = applyICF(*planeB++, m_icf[B]);

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

        Pixel pixel;
        if constexpr(MIX)
        {
            if((bp & 0xFF'00'00'00) == 0) // When mixing transparent pixels are black (V.5.9.1).
            {
                bp = BLACK_PIXEL;
            }

            if((fp & 0xFF'00'00'00) == 0)
            {
                fp = BLACK_PIXEL;
            }

            pixel = mix(fp, bp);
        }
        else // Overlay.
        {
            // Plane transparency is either 0 or 255.
            if((fp & 0xFF'00'00'00) == 0 && (bp & 0xFF'00'00'00) == 0) // Front and back plane transparent: only show background.
            {
                pixel = m_backdropColorARGB;
            }
            else if((fp & 0xFF'00'00'00) == 0) // Front plane transparent: show back plane.
            {
                pixel = bp;
            }
            else // Front plane visible: only show front plane.
            {
                pixel = fp;
            }
        }

        *screen++ = pixel;
    }
}

} // namespace Video
