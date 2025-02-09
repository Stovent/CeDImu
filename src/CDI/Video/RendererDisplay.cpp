#include "Renderer.hpp"

#include "../common/panic.hpp"
#include "../common/utils.hpp"

#include <cstring>
#if __SSE4_1__
#include <emmintrin.h>
#include <smmintrin.h>
#include <tmmintrin.h>
#endif // __SSE4_1__

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
std::pair<uint16_t, uint16_t> Renderer::DrawLine(const uint8_t* lineA, const uint8_t* lineB) noexcept
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
const Plane& Renderer::RenderFrame() noexcept
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
uint16_t Renderer::DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept
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

void Renderer::DrawLineBackdrop() noexcept
{
    // The pixels of a line are all the same, so backdrop plane only contains the color of each line.
    backdropColorToRGB(m_backdropPlane(m_lineNumber), m_backdropColor);
}

void Renderer::DrawCursor() noexcept
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
[[maybe_unused]] static constexpr uint8_t applyICFCPP(const int color, const int icf) noexcept
{
    return static_cast<uint8_t>(((icf * (color - 16)) / 63) + 16);
}

/** \brief Apply mixing to the given color components after ICF (V.5.9.1). */
[[maybe_unused]] static constexpr uint8_t mixCPP(const int a, const int b) noexcept
{
    return limu8(a + b - 16);
}

/** \brief Overlays or mix all the planes to the final screen using C++ code.
 * \tparam MIX true to use mixing, false to use overlay.
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 */
template<bool MIX, bool PLANE_ORDER>
static constexpr void overlayMixCPP(const uint8_t* planeA, const uint8_t* planeB, const uint8_t* background, uint32_t icfA, uint32_t icfB, uint8_t* screen) noexcept
{
    const uint8_t rbg = background[0];
    const uint8_t gbg = background[1];
    const uint8_t bbg = background[2];

    // for(uint16_t i = 0; i < m_plane[A].m_width; i++) // TODO: width[B].
    {
        const uint8_t aa = *planeA++;
        const uint8_t ra = applyICFCPP(*planeA++, icfA);
        const uint8_t ga = applyICFCPP(*planeA++, icfA);
        const uint8_t ba = applyICFCPP(*planeA++, icfA);

        const uint8_t ab = *planeB++;
        const uint8_t rb = applyICFCPP(*planeB++, icfB);
        const uint8_t gb = applyICFCPP(*planeB++, icfB);
        const uint8_t bb = applyICFCPP(*planeB++, icfB);

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

            r = mixCPP(rbp, rfp);
            g = mixCPP(gbp, gfp);
            b = mixCPP(bbp, bfp);
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

        screen[0] = r;
        screen[1] = g;
        screen[2] = b;
    }
}

#if __SSE4_1__
static constexpr std::array<int16_t, 8> SIXTEEN_ARRAY{0, 0, 16, 16, 16, 16, 16, 16};
static const __m128i SIXTEEN = _mm_loadu_si128(reinterpret_cast<const __m128i*>(SIXTEEN_ARRAY.data()));

/** \brief Applies ICF and mixes using SSE.
 */
static inline void applyICFMixSSE(const uint8_t* planeFront, const uint8_t* planeBack, uint32_t icfFront, uint32_t icfBack, uint8_t* screen) noexcept
{
    const __m128i icfF = _mm_set1_epi16(icfFront);
    const __m128i icfB = _mm_set1_epi16(icfBack);
    const __m128i icf = _mm_unpacklo_epi16(icfF, icfB);

    const __m128i xmmFront = _mm_loadu_si32(planeFront); // Load 32 bits in register (little endian).
    const __m128i xmmBack = _mm_loadu_si32(planeBack);

    __m128i xmm = _mm_unpacklo_epi8(xmmFront, xmmBack); // Interleave front and back plane bytes.

    xmm = _mm_cvtepu8_epi16(xmm); // Extend each componant from 8 to 16 bits.
    // xmm has front argb in bytes 0, 4, 8, 12 (as int16_t) and back argb in 2, 6, 10, 14.

    xmm = _mm_sub_epi16(xmm, SIXTEEN); // Subtract 16 from components.
    xmm = _mm_mullo_epi16(xmm, icf); // Multiply by ICF.
    xmm = _mm_srai_epi16(xmm, 6); // Divive by 63 (here 64 for performance).
    xmm = _mm_add_epi16(xmm, SIXTEEN); // Add 16 back.

    xmm = _mm_hadds_epi16(xmm, xmm); // Sum the components.
    // TODO: verify max and min levels.
    xmm = _mm_packus_epi16(xmm, xmm); // Convert back to uint8_t.

    const uint8_t old = screen[-1]; // TODO: is that UB with the first pixel being out of range?
    _mm_storeu_si32(&screen[-1], xmm);
    screen[-1] = old;
}

/** \brief Applies ICF and overlays using SSE.
 */
static constexpr void applyICFOverlaySSE(const uint8_t* planeFront, const uint8_t* planeBack, const uint8_t* background, uint32_t icfFront, uint32_t icfBack, uint8_t* screen) noexcept
{
    const uint8_t afp = planeFront[0];
    const uint8_t abp = planeBack[0];

    // Plane transparency is either 0 or 255.
    if(afp == 0 && abp == 0) [[unlikely]] // Front and back plane transparent: only show background.
    {
        memcpy(screen, background, 3);
        return;
    }

    __m128i xmm, icf;
    if(afp == 0) // Front plane transparent: show back plane.
    {
        icf = _mm_set1_epi16(icfBack);
        xmm = _mm_loadu_si32(planeBack); // Load 32 bits in register (little endian).
    }
    else // Front plane visible: only show front plane.
    {
        icf = _mm_set1_epi16(icfFront);
        xmm = _mm_loadu_si32(planeFront); // Load 32 bits in register (little endian).
    }

    xmm = _mm_cvtepu8_epi16(xmm); // Extend each componant from 8 to 16 bits.
    // xmm has front argb in bytes 0, 2, 4, 6 (as int16_t).

    xmm = _mm_sub_epi16(xmm, SIXTEEN); // Subtract 16 from components.
    xmm = _mm_mullo_epi16(xmm, icf); // Multiply by ICF.
    xmm = _mm_srai_epi16(xmm, 6); // Divive by 63 (here 64 for performance).
    xmm = _mm_add_epi16(xmm, SIXTEEN); // Add 16 back.

    xmm = _mm_packus_epi16(xmm, xmm); // Convert back to uint8_t.

    const uint8_t old = screen[-1]; // TODO: is that UB with the first pixel being out of range?
    _mm_storeu_si32(&screen[-1], xmm);
    screen[-1] = old;
}

/** \brief Overlays or mix all the planes to the final screen using x86 SSE4.1 optimisations.
 * \tparam MIX true to use mixing, false to use overlay.
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 */
template<bool MIX, bool PLANE_ORDER>
static constexpr void overlayMixSSE(const uint8_t* planeA, const uint8_t* planeB, const uint8_t* background, uint32_t icfA, uint32_t icfB, uint8_t* screen) noexcept
{
    if constexpr(MIX)
        if constexpr(PLANE_ORDER) // Plane B in front.
            applyICFMixSSE(planeB, planeA, icfB, icfA, screen);
        else // Plane A in front.
            applyICFMixSSE(planeA, planeB, icfA, icfB, screen);
    else // Overlay.
        if constexpr(PLANE_ORDER)
            applyICFOverlaySSE(planeB, planeA, background, icfB, icfA, screen);
        else // Plane A in front.
            applyICFOverlaySSE(planeA, planeB, background, icfA, icfB, screen);
}
#endif // __SSE4_1__

/** \brief Overlays or mix all the planes to the final screen.
 * \tparam MIX true to use mixing, false to use overlay.
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 */
template<bool MIX, bool PLANE_ORDER>
void Renderer::OverlayMix() noexcept
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

        // TODO: std::simd of C++26

#if __SSE4_1__
        overlayMixSSE<MIX, PLANE_ORDER>(planeA, planeB, background, m_icf[A], m_icf[B], screen);
#else
        overlayMixCPP<MIX, PLANE_ORDER>(planeA, planeB, background, m_icf[A], m_icf[B], screen);
#endif // __SSE4_1__

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

static constexpr uint32_t argbArrayToU32(uint8_t* pixels) noexcept
{
    return (as<uint32_t>(pixels[1]) << 16) | (as<uint32_t>(pixels[2]) << 8) | as<uint32_t>(pixels[3]);
}

/** \brief Handles the transparency of the current pixel for each plane.
 * \param pixel The ARGB pixel.
 */
template<Renderer::ImagePlane PLANE>
void Renderer::HandleTransparency(uint8_t pixel[4]) noexcept
{
    const bool boolean = !bit<3>(m_transparencyControl[PLANE]);
    uint32_t color = argbArrayToU32(pixel);
    color = clutColorKey(color | m_maskColorRgb[PLANE]);
    const bool colorKey = color == clutColorKey(m_transparentColorRgb[PLANE] | m_maskColorRgb[PLANE]); // TODO: don't compute if not CLUT.

    pixel[0] = PIXEL_FULL_INTENSITY;

    switch(m_transparencyControl[PLANE] & 0x07u)
    {
    case 0b000: // Always/Never.
        pixel[0] = PIXEL_FULL_INTENSITY + boolean; // Branchless.
        break;

    case 0b001: // Color Key.
        if(colorKey == boolean)
            pixel[0] = PIXEL_TRANSPARENT;
        break;

    case 0b010: // Transparent Bit.
        // TODO: currently decodeRGB555 make the pixel visible if the bit is set.
        // TODO: disable if not RGB555.
        if((pixel[0] == PIXEL_FULL_INTENSITY) != boolean)
            pixel[0] = PIXEL_TRANSPARENT;
        break;

    case 0b011: // Matte Flag 0.
        if(m_matteFlags[0] == boolean)
            pixel[0] = PIXEL_TRANSPARENT;
        break;

    case 0b100: // Matte Flag 1.
        if(m_matteFlags[1] == boolean)
            pixel[0] = PIXEL_TRANSPARENT;
        break;

    case 0b101: // Matte Flag 0 or Color Key.
        if(m_matteFlags[0] == boolean || colorKey == boolean)
            pixel[0] = PIXEL_TRANSPARENT;
        break;

    case 0b110: // Matte Flag 1 or Color Key.
        if(m_matteFlags[1] == boolean || colorKey == boolean)
            pixel[0] = PIXEL_TRANSPARENT;
        break;

    default: // Reserved.
        break;
    }
}

static constexpr bool matteMF(const uint32_t matteCommand) noexcept
{
    return bit<16>(matteCommand);
}

/** \brief Called at the beginning of each line to reset the matte state.
 */
void Renderer::ResetMatte() noexcept
{
    m_matteFlags.fill(false);

    if(!m_matteNumber) // One matte.
    {
        const bool matte = matteMF(m_matteControl[0]);
        m_nextMatte[matte] = 0;
        m_nextMatte[!matte] = m_matteControl.size();
    }
    else // Two mattes.
    {
        m_nextMatte[A] = 0;
        m_nextMatte[B] = MATTE_HALF;
    }
}

static constexpr uint8_t matteOp(const uint32_t matteCommand) noexcept
{
    return bits<20, 23>(matteCommand);
}

static constexpr uint8_t matteICF(const uint32_t matteCommand) noexcept
{
    return bits<10, 15>(matteCommand);
}

static constexpr uint16_t matteXPosition(const uint32_t matteCommand) noexcept
{
    return bits<1, 9>(matteCommand); // TODO: handle double resolution.
}

/** \brief Handles the matte flags for the given plane at the given pixel position.
 * \param pos The current pixel position (in normal resolution).
 */
template<Renderer::ImagePlane PLANE>
void Renderer::HandleMatte(const uint16_t pos) noexcept
{
    if(!m_matteNumber) // One matte.
    {
        if(m_nextMatte[PLANE] >= m_matteControl.size())
            return;
    }
    else // Two mattes.
    {
        if constexpr(PLANE == A)
        {
            if(m_nextMatte[A] >= MATTE_HALF)
                return;
        }
        else
            if(m_nextMatte[B] >= m_matteControl.size())
                return;
    }

    const uint32_t command = m_matteControl[m_nextMatte[PLANE]];
    if(matteXPosition(command) > pos)
        return;

    ++m_nextMatte[PLANE];

    const uint8_t op = matteOp(command);
    switch(op)
    {
    case 0b0000:
        m_nextMatte[PLANE] = m_matteControl.size();
        break;

    case 0b0100:
        m_icf[A] = matteICF(command);
        break;

    case 0b0110:
        m_icf[B] = matteICF(command);
        break;

    case 0b1000:
        m_matteFlags[PLANE] = false;
        break;

    case 0b1001:
        m_matteFlags[PLANE] = true;
        break;

    case 0b1100:
        m_icf[A] = matteICF(command);
        m_matteFlags[PLANE] = false;
        break;

    case 0b1101:
        m_icf[A] = matteICF(command);
        m_matteFlags[PLANE] = true;
        break;

    case 0b1110:
        m_icf[B] = matteICF(command);
        m_matteFlags[PLANE] = false;
        break;

    case 0b1111:
        m_icf[B] = matteICF(command);
        m_matteFlags[PLANE] = true;
        break;
    }
}

} // namespace Video
