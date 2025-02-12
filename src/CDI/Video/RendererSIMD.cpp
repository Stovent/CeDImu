#include "RendererSIMD.hpp"

#include "../common/panic.hpp"
#include "../common/utils.hpp"
#include "../common/VideoSIMD.hpp"

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
std::pair<uint16_t, uint16_t> RendererSIMD::DrawLine(const uint8_t* lineA, const uint8_t* lineB) noexcept
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
const Plane& RendererSIMD::RenderFrame() noexcept
{
    // Should this be inside DrawLine() ?
    if(m_cursorEnabled)
    {
        DrawCursor();
        Video::paste(m_screen.data(), m_screen.m_width, m_screen.m_height, m_cursorPlane.data(), m_cursorPlane.m_width, m_cursorPlane.m_height, m_cursorX, m_cursorY);
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
uint16_t RendererSIMD::DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept
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
        return decodeBitmapLineSIMD(m_planeLine[PLANE].data(), lineA, lineMain, m_plane[PLANE].m_width, clut, m_dyuvInitialValue[PLANE], m_codingMethod[PLANE]);

    case ImageType::RunLength:
        return decodeRunLengthLineSIMD(m_plane[PLANE](m_lineNumber), lineMain, m_plane[PLANE].m_width, clut, is4BPP);

    case ImageType::Mosaic:
        panic("Unsupported type Mosaic");
        return 0;
    }

    std::unreachable();
}

void RendererSIMD::DrawLineBackdrop() noexcept
{
    // The pixels of a line are all the same, so backdrop plane only contains the color of each line.
    backdropColorToRGB(m_backdropPlane(m_lineNumber), m_backdropColor);
}

void RendererSIMD::DrawCursor() noexcept
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

// int32_t generates a bit shorter code than int16_t.
using Elements = stdx::fixed_size_simd<int32_t, 4>;

static const Elements SIXTEEN{16};

/** \brief Applies ICF and mixes using SIMD.
 * \param icfs The front ICF in low 16 bits and the back ICF in high 16 bits
 */
static inline void applyICFMixSIMD(const uint8_t* planeFront, const uint8_t* planeBack, int32_t icfFront, int32_t icfBack, uint8_t* screen) noexcept
{
    const Elements icfF{icfFront};
    const Elements icfB{icfBack};

    Elements planeF{planeFront, stdx::element_aligned};
    Elements planeB{planeBack, stdx::element_aligned};

    planeF -= SIXTEEN;
    planeB -= SIXTEEN;
    planeF *= icfF;
    planeB *= icfB;
    planeF >>= 6;
    planeB >>= 6;
    planeF += SIXTEEN;
    planeB += SIXTEEN;

    Elements result{planeF + planeB};
    result -= SIXTEEN;

    const uint8_t old = screen[-1];
    result.copy_to(&screen[-1], stdx::element_aligned);
    screen[-1] = old;
    // result.copy_to(screen, stdx::element_aligned);
}

/** \brief Applies ICF and overlays using SIMD implementation.
 */
static inline void applyICFOverlayPlaneSIMD(const uint8_t* plane, uint8_t icf, uint8_t* screen) noexcept
{
    Elements elements{plane, stdx::element_aligned};
    const Elements simdIcf = icf;

    elements -= SIXTEEN;
    elements *= simdIcf;
    elements >>= 6;
    elements += SIXTEEN;

    const uint8_t old = screen[-1];
    elements.copy_to(&screen[-1], stdx::element_aligned);
    screen[-1] = old;
    // elements.copy_to(screen, stdx::element_aligned);
}

/** \brief Applies ICF and overlays using SIMD, dispatching to the most efficient implementation.
 */
static constexpr void applyICFOverlaySIMD(const uint8_t* planeFront, const uint8_t* planeBack, const uint8_t* background, uint8_t icfFront, uint8_t icfBack, uint8_t* screen) noexcept
{
    const uint8_t afp = planeFront[0];
    const uint8_t abp = planeBack[0];

    // Plane transparency is either 0 or 255.
    if(afp == 0 && abp == 0) [[unlikely]] // Front and back plane transparent: only show background.
        memcpy(screen, background, 3);
    else if(afp == 0) // Front plane transparent: show back plane.
        applyICFOverlayPlaneSIMD(planeBack, icfBack, screen);
    else // Front plane visible: only show front plane.
        applyICFOverlayPlaneSIMD(planeFront, icfFront, screen);
}

/** \brief Overlays or mix all the planes to the final screen using x86 SSE4.1 optimisations.
 * \tparam MIX true to use mixing, false to use overlay.
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 */
template<bool MIX, bool PLANE_ORDER>
static constexpr void overlayMixSIMD(const uint8_t* planeA, const uint8_t* planeB, const uint8_t* background, uint32_t icfA, uint32_t icfB, uint8_t* screen) noexcept
{
    if constexpr(MIX)
        if constexpr(PLANE_ORDER) // Plane B in front.
            applyICFMixSIMD(planeB, planeA, icfB, icfA, screen);
        else // Plane A in front.
            applyICFMixSIMD(planeA, planeB, icfA, icfB, screen);
    else // Overlay.
        if constexpr(PLANE_ORDER)
            applyICFOverlaySIMD(planeB, planeA, background, icfB, icfA, screen);
        else // Plane A in front.
            applyICFOverlaySIMD(planeA, planeB, background, icfA, icfB, screen);
}

/** \brief Overlays or mix all the planes to the final screen.
 * \tparam MIX true to use mixing, false to use overlay.
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 */
template<bool MIX, bool PLANE_ORDER>
void RendererSIMD::OverlayMix() noexcept
{
    uint8_t* screen = m_screen(m_lineNumber);
    uint8_t* planeA = m_plane[A](m_lineNumber);
    uint8_t* planeB = m_plane[B](m_lineNumber);
    uint8_t* background = m_backdropPlane(m_lineNumber);

    for(uint16_t i = 0; i < m_plane[A].m_width; i++) // TODO: width[B].
    {
        HandleMatte<A>(i);
        HandleMatte<B>(i);

        HandleTransparency<A>(planeA);
        HandleTransparency<B>(planeB);

        overlayMixSIMD<MIX, PLANE_ORDER>(planeA, planeB, background, m_icf[A], m_icf[B], screen);

        planeA += 4;
        planeB += 4;
        screen += 3;

        /*
        MCD212 figure 8-6
        For overlay/mix:
        - do the computation for a single pixel.
        - write it to the destination buffer with memcpy for mosaic and double resolution.
        */
    }
}

} // namespace Video
