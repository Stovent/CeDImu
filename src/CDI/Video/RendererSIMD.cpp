#include "RendererSIMD.hpp"

#include "../common/panic.hpp"
#include "../common/utils.hpp"
#include "VideoSIMD.hpp"
#include "VideoDecoders.hpp"

#include <bit>
#include <cstring>
#include <execution>

namespace Video
{

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
        // TODO: double resolution
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
uint16_t RendererSIMD::DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept
{
    if(m_codingMethod[PLANE] == ImageCodingMethod::OFF)
    {
        std::fill_n(std::execution::unseq, m_planeLine[PLANE].begin(), m_plane[PLANE].m_width, 0);
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
    *m_backdropPlane.GetLinePointer(m_lineNumber) = backdropCursorColorToPixel(m_backdropColor);
}

void RendererSIMD::DrawCursor() noexcept
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

    // TODO: try in SIMD.
//     using FixedPixelSIMD = stdx::fixed_size_simd<uint32_t, 16>;
//     int pattern = 0;
//     for(PlaneSIMD::iterator it = m_cursorPlane.begin(); it < m_cursorPlane.end(); it += FixedPixelSIMD::size(), ++pattern)
//     {
//         FixedPixelSIMD pixel{&*it, stdx::element_aligned};
//
//         for(int x = m_cursorPlane.m_width - 1, pix = 0; --x >= 0; pix++)
//         {
//             const uint16_t mask = (1 << x);
//             if(m_cursorPatterns[pattern] & mask)
//                 pixel[pix] = color;
//             else
//                 pixel[pix] = black;
//         }
//         pixel.copy_to(&*it, stdx::element_aligned);
//     }
}

/** \brief Overlays or mix all the planes to the final screen.
 * \tparam MIX true to use mixing, false to use overlay.
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 */
template<bool MIX, bool PLANE_ORDER>
void RendererSIMD::OverlayMix() noexcept
{
    PixelU32* planeA = m_planeLine[A].data();
    PixelU32* planeB = m_planeLine[B].data();

    for(uint16_t i = 0; i < m_plane[A].m_width; ++i) // TODO: width[B].
    {
        HandleMatte<A>(i);
        HandleMatte<B>(i);
        // These two lines below adds a little noticable delay.
        m_icfLine[A][i] = m_icf[A];
        m_icfLine[B][i] = m_icf[B];
        HandleTransparencySIMD<A>(*planeA++);
        HandleTransparencySIMD<B>(*planeB++);
    }

    HandleOverlayMixSIMD<MIX, PLANE_ORDER>();
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
// template<typename SIMD>
static constexpr void applyICFMixSIMDShift(Pixel* screen, const PixelU32* planeFront, const PixelU32* planeBack, const uint8_t* icfFront, const uint8_t* icfBack) noexcept
{
    PixelSIMDSigned icfF{icfFront, stdx::element_aligned};
    PixelSIMDSigned icfB{icfBack, stdx::element_aligned};

    PixelSIMDSigned planeF{planeFront, stdx::element_aligned};
    PixelSIMDSigned planeB{planeBack, stdx::element_aligned};

    // transparent areas of an image simply give no contribution to the final display
    // - that is they are equivalent to black areas.
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

//     rfp >>= 6;
//     gfp >>= 6;
//     bfp >>= 6;
//
//     rbp >>= 6;
//     gbp >>= 6;
//     bbp >>= 6;

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

    result.copy_to(reinterpret_cast<PixelU32*>(screen), stdx::element_aligned);
}
// template void applyICFMixSIMDShift<false>() noexcept;
// template void applyICFMixSIMDShift<true>() noexcept;

/** \brief Applies ICF and mixes using SIMD (algorithm that casts the registers to access RGB components).
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 */
// template<typename SIMD>
static constexpr void applyICFMixSIMDCast(Pixel* screen, const PixelU32* planeFront, const PixelU32* planeBack, const uint8_t* icfFront, const uint8_t* icfBack) noexcept
{
    PixelSIMD icfF{icfFront, stdx::element_aligned};
    PixelSIMD icfB{icfBack, stdx::element_aligned};

    PixelSIMD planeF{planeFront, stdx::element_aligned};
    PixelSIMD planeB{planeBack, stdx::element_aligned};

    // transparent areas of an image simply give no contribution to the final display
    // - that is they are equivalent to black areas.
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

    // rgbF16 >>= 6;
    // rgbB16 >>= 6;

    rgbF16 += SIXTEENN;
    // rgbB16 += SIXTEENN; Don't add 16 to back plane when applying ICF because the below mixing subtracts it.

    rgbF16 += rgbB16;

    rgbF16 = stdx::clamp(rgbF16, U8_MINN, U8_MAXX);

    const PixelSIMD result = std::bit_cast<PixelSIMD>(stdx::static_simd_cast<SIMDU8>(rgbF16));

    result.copy_to(reinterpret_cast<PixelU32*>(screen), stdx::element_aligned);
}
// template void applyICFMixSIMDCast<false>() noexcept;
// template void applyICFMixSIMDCast<true>() noexcept;

/** \brief Applies ICF and overlays using SIMD.
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 *
 * TODO: implement the cast method here too and benchmark it.
 */
// template<typename SIMD>
static constexpr void applyICFOverlaySIMD(Pixel* screen, const PixelU32* planeFront, const PixelU32* planeBack, const uint8_t* icfFront, const uint8_t* icfBack, const uint32_t backdrop) noexcept
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

//     rfp >>= 6;
//     gfp >>= 6;
//     bfp >>= 6;
//
//     rbp >>= 6;
//     gbp >>= 6;
//     bbp >>= 6;

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
    stdx::where(maskFrontBackZero, result) = backdrop;

    result.copy_to(reinterpret_cast<PixelU32*>(screen), stdx::element_aligned);
}

/** \brief Dispatches the correct over or mix SIMD algorithm.
 * \tparam MIX true to use mixing, false to use overlay.
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 */
template<bool MIX, bool PLANE_ORDER>
void RendererSIMD::HandleOverlayMixSIMD() noexcept
{
    Pixel* screen = m_screen.GetLinePointer(m_lineNumber);
    const PixelU32* planeFront;
    const PixelU32* planeBack;
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

    for(int i = static_cast<int>(m_plane[A].m_width); i > 0;
        i -= SIMD_SIZE, planeFront += SIMD_SIZE, planeBack += SIMD_SIZE, icfFront += SIMD_SIZE, icfBack += SIMD_SIZE, screen += SIMD_SIZE) // TODO: width[B].
    {
        if constexpr(MIX) // Mixing.
            applyICFMixSIMDShift(screen, planeFront, planeBack, icfFront, icfBack);
            // applyICFMixSIMDCast(screen, planeFront, planeBack, icfFront, icfBack);
        else // Overlay.
            applyICFOverlaySIMD(screen, planeFront, planeBack, icfFront, icfBack, m_backdropPlane.GetLinePointer(m_lineNumber)->AsU32());
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
