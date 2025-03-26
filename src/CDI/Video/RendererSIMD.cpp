#include "RendererSIMD.hpp"

#include "../common/panic.hpp"
#include "../common/utils.hpp"
#include "../common/VideoSIMD.hpp"

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
std::pair<uint16_t, uint16_t> RendererSIMD::DrawLine(const uint8_t* lineA, const uint8_t* lineB) noexcept
{
    ResetMatteSIMD();

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

    // TODO: this should be on the GUI side.
    Plane::iterator dstit = m_screen.begin();
    PlaneSIMD::const_iterator it = m_screenARGB.cbegin();
    for(size_t i = 0; i < m_screenARGB.PixelCount(); ++i, ++it, dstit += 3)
    {
        const Pixel pixel = *it;
        dstit[0] = pixel >> 16;
        dstit[1] = pixel >> 8;
        dstit[2] = pixel;
    }

//     using FixedSIMD8 = stdx::fixed_size_simd<uint8_t, PixelSIMD::size()>;
//     for(size_t i = 0; i < m_screenARGB.PixelCount(); ++i, it += PixelSIMD::size(), dstit += 3 /** FixedSIMD8::size()*/)
//     {
//         const PixelSIMD p{&*it, stdx::element_aligned};
//
//         const FixedSIMD8 r = stdx::static_simd_cast<uint8_t>(p >> 16);
//         const FixedSIMD8 g = stdx::static_simd_cast<uint8_t>(p >> 8);
//         const FixedSIMD8 b = stdx::static_simd_cast<uint8_t>(p);
//
//         // scalar ? or array.
//         r.copy_to(&*dstit, stdx::element_aligned);
//         g.copy_to(&*dstit, stdx::element_aligned);
//         b.copy_to(&*dstit, stdx::element_aligned);
//     }

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
        return decodeRunLengthLineSIMD(m_planeLine[PLANE].data(), lineMain, m_plane[PLANE].m_width, clut, is4BPP);

    case ImageType::Mosaic:
        panic("Unsupported type Mosaic");
        return 0;
    }

    std::unreachable();
}

void RendererSIMD::DrawLineBackdrop() noexcept
{
    // The pixels of a line are all the same, so backdrop plane only contains the color of each line.
    m_backdropColorARGB = backdropCursorColorToARGB(m_backdropColor);
}

void RendererSIMD::DrawCursor() noexcept
{
    // Technically speaking the cursor is drawn when the drawing line number is the cursor's one (because video
    // is outputted continuously line by line).
    // But for here maybe we don't care.
    using FixedPixelSIMD = stdx::fixed_size_simd<uint32_t, 16>;

    const uint32_t color = backdropCursorColorToARGB(m_cursorColor);
    const uint32_t black{0};

    int pattern = 0;
    // TODO: benchmark, that may not be faster at all.
    for(PlaneSIMD::iterator it = m_cursorPlaneARGB.begin(); it < m_cursorPlaneARGB.end(); it += FixedPixelSIMD::size(), ++pattern)
    {
        FixedPixelSIMD pixel{&*it, stdx::element_aligned};

        for(int x = m_cursorPlaneARGB.m_width, pix = 0; --x >= 0; pix++)
        {
            const uint16_t mask = (1 << x);
            if(m_cursorPatterns[pattern] & mask)
                pixel[pix] = color;
            else
                pixel[pix] = black;
        }
        pixel.copy_to(&*it, stdx::element_aligned);
    }
}

/** \brief Overlays or mix all the planes to the final screen.
 * \tparam MIX true to use mixing, false to use overlay.
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 */
template<bool MIX, bool PLANE_ORDER>
void RendererSIMD::OverlayMix() noexcept
{
    Pixel* planeA = m_planeLine[A].data();
    Pixel* planeB = m_planeLine[B].data();

    for(uint16_t i = 0; i < m_plane[A].m_width; ++i) // TODO: width[B].
    {
        HandleMatte<A>(i);
        HandleMatte<B>(i);
        m_icfLine[A][i] = m_icf[A];
        m_icfLine[B][i] = m_icf[B];
        HandleTransparencySIMD<A>(*planeA++);
        HandleTransparencySIMD<B>(*planeB++);
    }

    if constexpr(MIX) // Mixing.
        ApplyICFMixSIMDShift<PLANE_ORDER>();
        // ApplyICFMixSIMDCast<PLANE_ORDER>();
    else // Overlay.
        ApplyICFOverlaySIMD<PLANE_ORDER>();
}

using PixelSIMDSigned = stdx::native_simd<int32_t>;
using SIMDU8 = stdx::native_simd<uint8_t>;
using SIMDS16 = stdx::native_simd<int16_t>;
using FixedS16 = stdx::fixed_size_simd<int16_t, SIMDS16::size() * sizeof(SIMDS16::value_type)>;

static const PixelSIMDSigned SIXTEEN{16};
static const FixedS16 SIXTEENN{16};
static const PixelSIMDSigned U8_MIN{0};
static const FixedS16 U8_MINN{0};
static const PixelSIMDSigned U8_MAX{255};
static const FixedS16 U8_MAXX{255};
static const PixelSIMDSigned ALPHA_MASK{-16777216}; // 0xFF'00'00'00
static const PixelSIMD ALPHA_MASKK{0xFF'00'00'00}; // 0xFF'00'00'00

/** \brief Applies ICF and mixes using SIMD (algorithm that shifts and masks RGB components).
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 */
template<bool PLANE_ORDER>
void RendererSIMD::ApplyICFMixSIMDShift() noexcept
{
    Pixel* screen = m_screenARGB.GetLinePointer(m_lineNumber);
    const Pixel* planeFront;
    const Pixel* planeBack;
    const uint8_t* icfFront;
    const uint8_t* icfBack;
    if constexpr(PLANE_ORDER)
    {
        planeFront = m_planeLine[B].data();
        planeBack = m_planeLine[A].data();
        icfFront = m_icfLine[B].data();
        icfBack = m_icfLine[A].data();
    }
    else
    {
        planeFront = m_planeLine[A].data();
        planeBack = m_planeLine[B].data();
        icfFront = m_icfLine[A].data();
        icfBack = m_icfLine[B].data();
    }

    for(uint16_t i = 0; i < m_plane[A].m_width;
        i += SIMD_SIZE, planeFront += SIMD_SIZE, planeBack += SIMD_SIZE, icfFront += SIMD_SIZE, icfBack += SIMD_SIZE, screen += SIMD_SIZE) // TODO: width[B].
    {
        PixelSIMDSigned icfF{icfFront, stdx::element_aligned};
        PixelSIMDSigned icfB{icfBack, stdx::element_aligned};

        PixelSIMDSigned planeF{planeFront, stdx::element_aligned};
        PixelSIMDSigned planeB{planeBack, stdx::element_aligned};

        // transparent areas of an image simply give no contribution to the final display
        // - that is they are equivalent to black areas..
        const PixelSIMDSigned::mask_type maskF = (planeF & ALPHA_MASK) == 0;
        const PixelSIMDSigned::mask_type maskB = (planeB & ALPHA_MASK) == 0;
        stdx::where(maskF, planeF) = 0x00'10'10'10;
        stdx::where(maskB, planeB) = 0x00'10'10'10;
        stdx::where(maskF, icfF) = 63;
        stdx::where(maskB, icfB) = 63;

        PixelSIMDSigned rfp = planeF >> 16 & 0xFF;
        PixelSIMDSigned gfp = planeF >> 8 & 0xFF;
        PixelSIMDSigned bfp = planeF & 0xFF;

        PixelSIMDSigned rbp = planeB >> 16 & 0xFF;
        PixelSIMDSigned gbp = planeB >> 8 & 0xFF;
        PixelSIMDSigned bbp = planeB & 0xFF;

        rfp -= SIXTEEN;
        gfp -= SIXTEEN;
        bfp -= SIXTEEN;

        rbp -= SIXTEEN;
        gbp -= SIXTEEN;
        bbp -= SIXTEEN;

        rfp *= icfF;
        gfp *= icfF;
        bfp *= icfF;

        rbp *= icfB;
        gbp *= icfB;
        bbp *= icfB;

        rfp /= 63;
        gfp /= 63;
        bfp /= 63;

        rbp /= 63;
        gbp /= 63;
        bbp /= 63;

//         rfp >>= 6;
//         gfp >>= 6;
//         bfp >>= 6;
//
//         rbp >>= 6;
//         gbp >>= 6;
//         bbp >>= 6;

        rfp += SIXTEEN;
        gfp += SIXTEEN;
        bfp += SIXTEEN;
        // Don't add 16 to back plane when applying ICF because the below mixing subtracts it.

        rfp += rbp;
        gfp += gbp;
        bfp += bbp;

        rfp = stdx::clamp(rfp, U8_MIN, U8_MAX);
        gfp = stdx::clamp(gfp, U8_MIN, U8_MAX);
        bfp = stdx::clamp(bfp, U8_MIN, U8_MAX);

        const PixelSIMDSigned result = (rfp << 16) | (gfp << 8) | bfp;

        result.copy_to(screen, stdx::element_aligned);
    }
}
// template void RendererSIMD::ApplyICFMixSIMDShift<false>() noexcept;
// template void RendererSIMD::ApplyICFMixSIMDShift<true>() noexcept;

/** \brief Applies ICF and mixes using SIMD (algorithm that casts the registers to access RGB components).
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 */
template<bool PLANE_ORDER>
void RendererSIMD::ApplyICFMixSIMDCast() noexcept
{
    Pixel* screen = m_screenARGB.GetLinePointer(m_lineNumber);
    const Pixel* planeFront;
    const Pixel* planeBack;
    const uint8_t* icfFront;
    const uint8_t* icfBack;
    if constexpr(PLANE_ORDER)
    {
        planeFront = m_planeLine[B].data();
        planeBack = m_planeLine[A].data();
        icfFront = m_icfLine[B].data();
        icfBack = m_icfLine[A].data();
    }
    else
    {
        planeFront = m_planeLine[A].data();
        planeBack = m_planeLine[B].data();
        icfFront = m_icfLine[A].data();
        icfBack = m_icfLine[B].data();
    }

    for(uint16_t i = 0; i < m_plane[A].m_width;
        i += SIMD_SIZE, planeFront += SIMD_SIZE, planeBack += SIMD_SIZE, icfFront += SIMD_SIZE, icfBack += SIMD_SIZE, screen += SIMD_SIZE) // TODO: width[B].
    {
        PixelSIMD icfF{icfFront, stdx::element_aligned};
        PixelSIMD icfB{icfBack, stdx::element_aligned};

        PixelSIMD planeF{planeFront, stdx::element_aligned};
        PixelSIMD planeB{planeBack, stdx::element_aligned};

        // transparent areas of an image simply give no contribution to the final display
        // - that is they are equivalent to black areas..
        const PixelSIMD::mask_type maskF = (planeF & ALPHA_MASKK) == 0;
        const PixelSIMD::mask_type maskB = (planeB & ALPHA_MASKK) == 0;
        stdx::where(maskF, planeF) = 0x00'10'10'10;
        stdx::where(maskB, planeB) = 0x00'10'10'10;
        stdx::where(maskF, icfF) = 63;
        stdx::where(maskB, icfB) = 63;

        // extend ICF to whole register.
        icfF *= 0x00'01'01'01;
        icfB *= 0x00'01'01'01;
        // icfF |= (icfF << 16) | (icfF << 8);
        // icfB |= (icfB << 16) | (icfB << 8);

        SIMDU8 rgbF8 = std::bit_cast<SIMDU8>(planeF);
        SIMDU8 rgbB8 = std::bit_cast<SIMDU8>(planeB);
        SIMDU8 icfF8 = std::bit_cast<SIMDU8>(icfF);
        SIMDU8 icfB8 = std::bit_cast<SIMDU8>(icfB);

        FixedS16 rgbF16 = stdx::static_simd_cast<int16_t>(rgbF8);
        FixedS16 rgbB16 = stdx::static_simd_cast<int16_t>(rgbB8);
        FixedS16 icfF16 = stdx::static_simd_cast<int16_t>(icfF8);
        FixedS16 icfB16 = stdx::static_simd_cast<int16_t>(icfB8);

        rgbF16 -= SIXTEENN;
        rgbB16 -= SIXTEENN;

        rgbF16 *= icfF16;
        rgbB16 *= icfB16;

        rgbF16 /= 63;
        rgbB16 /= 63;

        rgbF16 += SIXTEENN;
        // rgbB16 += SIXTEENN; Don't add 16 to back plane when applying ICF because the below mixing subtracts it.

        rgbF16 += rgbB16;

        rgbF16 = stdx::clamp(rgbF16, U8_MINN, U8_MAXX);

        const PixelSIMD result = std::bit_cast<PixelSIMD>(stdx::static_simd_cast<SIMDU8>(rgbF16));

        result.copy_to(screen, stdx::element_aligned);
    }
}
// template void RendererSIMD::ApplyICFMixSIMDCast<false>() noexcept;
// template void RendererSIMD::ApplyICFMixSIMDCast<true>() noexcept;

/** \brief Applies ICF and overlays using SIMD.
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 *
 * TODO: implement the cast method here too and benchmark it.
 */
template<bool PLANE_ORDER>
void RendererSIMD::ApplyICFOverlaySIMD() noexcept
{
    Pixel* screen = m_screenARGB.GetLinePointer(m_lineNumber);
    const Pixel* planeFront;
    const Pixel* planeBack;
    const uint8_t* icfFront;
    const uint8_t* icfBack;
    if constexpr(PLANE_ORDER)
    {
        planeFront = m_planeLine[B].data();
        planeBack = m_planeLine[A].data();
        icfFront = m_icfLine[B].data();
        icfBack = m_icfLine[A].data();
    }
    else
    {
        planeFront = m_planeLine[A].data();
        planeBack = m_planeLine[B].data();
        icfFront = m_icfLine[A].data();
        icfBack = m_icfLine[B].data();
    }

    for(uint16_t i = 0; i < m_plane[A].m_width;
        i += SIMD_SIZE, planeFront += SIMD_SIZE, planeBack += SIMD_SIZE, icfFront += SIMD_SIZE, icfBack += SIMD_SIZE, screen += SIMD_SIZE) // TODO: width[B].
    {
        PixelSIMDSigned planeF{planeFront, stdx::element_aligned};
        PixelSIMDSigned planeB{planeBack, stdx::element_aligned};
        const PixelSIMDSigned icfF{icfFront, stdx::element_aligned};
        const PixelSIMDSigned icfB{icfBack, stdx::element_aligned};

        const PixelSIMDSigned afp = planeF >> 24 & 0xFF;
        PixelSIMDSigned rfp = planeF >> 16 & 0xFF;
        PixelSIMDSigned gfp = planeF >> 8 & 0xFF;
        PixelSIMDSigned bfp = planeF & 0xFF;

        const PixelSIMDSigned abp = planeB >> 24 & 0xFF;
        PixelSIMDSigned rbp = planeB >> 16 & 0xFF;
        PixelSIMDSigned gbp = planeB >> 8 & 0xFF;
        PixelSIMDSigned bbp = planeB & 0xFF;

        rfp -= SIXTEEN;
        gfp -= SIXTEEN;
        bfp -= SIXTEEN;

        rbp -= SIXTEEN;
        gbp -= SIXTEEN;
        bbp -= SIXTEEN;

        rfp *= icfF;
        gfp *= icfF;
        bfp *= icfF;

        rbp *= icfB;
        gbp *= icfB;
        bbp *= icfB;

        rfp /= 63;
        gfp /= 63;
        bfp /= 63;

        rbp /= 63;
        gbp /= 63;
        bbp /= 63;

//         rfp >>= 6;
//         gfp >>= 6;
//         bfp >>= 6;
//
//         rbp >>= 6;
//         gbp >>= 6;
//         bbp >>= 6;

        rfp += SIXTEEN;
        gfp += SIXTEEN;
        bfp += SIXTEEN;

        rbp += SIXTEEN;
        gbp += SIXTEEN;
        bbp += SIXTEEN;

        planeF = (rfp << 16) | (gfp << 8) | bfp;
        planeB = (rbp << 16) | (gbp << 8) | bbp;

        PixelSIMDSigned result{planeF};
        const PixelSIMDSigned::mask_type maskFrontZero = afp == 0;
        stdx::where(maskFrontZero, result) = planeB;
        const PixelSIMDSigned::mask_type maskFrontBackZero = maskFrontZero && abp == 0;
        stdx::where(maskFrontBackZero, result) = m_backdropColorARGB;

        result.copy_to(screen, stdx::element_aligned);
    }
}

/** \brief Called at the beginning of each line to reset the matte state.
 *
 * TODO: how does ICF behave after the frame?
 * - is it reset to m_icf[A/B] on each line?
 * - does it keep the latest value for all the next line? (so m_icfLine[A/B].fill(m_icfLine[A/B][last]);)
 */
void RendererSIMD::ResetMatteSIMD() noexcept
{
    ResetMatte();

    m_icfLine[A].fill(m_icf[A]);
    m_icfLine[B].fill(m_icf[B]);
}

} // namespace Video
