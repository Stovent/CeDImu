/** \file RendererSIMD.cpp
 * \brief Implementation of RendererSIMD class.
 *
 * The Cast version of Overlay/Mix should be faster than Shift, but previous benchmarks showed that Cast was slower on
 * AVX2, but I can't reproduce it now. So Cast is used for now.
 *
 * All these functions does not seem to benefit from factorizing the applyICF to a dedicated function, as MixSIMDCast
 * uses the intermediate representation for the computation, and OverlaySIMDCast reuses the mask from ICF computation.
 */

#include "RendererSIMD.hpp"

#include "../common/panic.hpp"
#include "../common/utils.hpp"
#include "SIMD.hpp"
#include "VideoDecoders.hpp"
#include "VideoDecodersSIMD.hpp"

#include <bit>
#include <cstring>
#include <execution>
#include <utility>

namespace Video
{

/** \brief Draws the next line to draw.
 * \param lineA Line A data.
 * \param lineB Line B data.
 * \param lineNumber The line number to draw (starting at 0).
 * \return The number of bytes read from memory for each plane `<plane A, plane B>`.
 */
std::pair<uint16_t, uint16_t> RendererSIMD::DrawLine(const uint8_t* lineA, const uint8_t* lineB, const uint16_t lineNumber) noexcept
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
uint16_t RendererSIMD::DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept
{
    if(m_codingMethod[PLANE] == ImageCodingMethod::OFF)
    {
        std::fill_n(std::execution::unseq, m_plane[PLANE].GetLinePointer(m_lineNumber), m_plane[PLANE].m_width, 0);
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
                return decodeBitmapLineSIMD<720>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineA, lineMain, clut, m_dyuvInitialValue[PLANE], icm);
            else
                return decodeBitmapLineSIMD<768>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineA, lineMain, clut, m_dyuvInitialValue[PLANE], icm);
        else
            if(Is360Pixels())
                return decodeBitmapLineSIMD<360>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineA, lineMain, clut, m_dyuvInitialValue[PLANE], icm);
            else
                return decodeBitmapLineSIMD<384>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineA, lineMain, clut, m_dyuvInitialValue[PLANE], icm);

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

void RendererSIMD::DrawCursor() noexcept
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

    using SIMDCursorLine = stdx::fixed_size_simd<uint32_t, 16>;
    using SIMDCursorLineMask = SIMDCursorLine::mask_type;
    static constexpr SIMDCursorLine SHIFTER([] (uint32_t i) { return 15 - i; });

    const SIMDCursorLine colorSimd{color.AsU32()};
    int patternIndex = 0;
    for(Plane::iterator dst = m_cursorPlane.begin(); dst < m_cursorPlane.end(); dst += SIMDCursorLine::size(), ++patternIndex)
    {
        // Convert pattern to a mask.
        SIMDCursorLine patternSimd = m_cursorPatterns[patternIndex];
        patternSimd >>= SHIFTER;
        patternSimd &= 1;
        const SIMDCursorLineMask patternMask = patternSimd == 1;

        SIMDCursorLine cursorPixels = BLACK_PIXEL.AsU32();
        stdx::where(patternMask, cursorPixels) = colorSimd;
        cursorPixels.copy_to(dst->AsU32Pointer(), stdx::element_aligned);
    }
}

/** \brief Overlays or mix all the planes to the final screen.
 * \tparam MIX true to use mixing, false to use overlay.
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 */
template<bool MIX, bool PLANE_ORDER>
void RendererSIMD::OverlayMix() noexcept
{
    HandleMatteAndTransparency(m_lineNumber);

    switch(m_plane[A].m_width)
    {
    case 720:
        HandleOverlayMixSIMD<MIX, PLANE_ORDER, SIMDReminder<720>::value>();
        break;

    case 768:
        HandleOverlayMixSIMD<MIX, PLANE_ORDER, SIMDReminder<768>::value>();
        break;

    default:
        std::unreachable();
    }
}

static inline constexpr SIMDNativePixelSigned U8_MIN{0};
static inline constexpr SIMDFixedS16 U8_MINN{0};
static inline constexpr SIMDNativePixelSigned U8_MAX{255};
static inline constexpr SIMDFixedS16 U8_MAXX{255};
static inline constexpr SIMDNativePixelSigned ALPHA_MASK{-16777216}; // 0xFF'00'00'00
static inline constexpr SIMDNativePixel ALPHA_MASKK{0xFF'00'00'00};

/** \brief Applies ICF and mixes using SIMD (algorithm that shifts and masks RGB components).
 * \tparam SIMD The SIMD type holding signed 32 bits integers.
 */
template<typename SIMD>
static constexpr void applyICFMixSIMDShift(Pixel* screen, const Pixel* planeFront, const Pixel* planeBack, const uint8_t* icfFront, const uint8_t* icfBack) noexcept
{
    SIMD icfF{icfFront, stdx::element_aligned};
    SIMD icfB{icfBack, stdx::element_aligned};

    SIMD planeF{planeFront->AsU32Pointer(), stdx::element_aligned};
    SIMD planeB{planeBack->AsU32Pointer(), stdx::element_aligned};

    // transparent areas of an image simply give no contribution to the final display
    // - that is they are equivalent to black areas.
    const typename SIMD::mask_type maskF = (planeF & ALPHA_MASK) == 0;
    const typename SIMD::mask_type maskB = (planeB & ALPHA_MASK) == 0;
    stdx::where(maskF, planeF) = 0x00'10'10'10;
    stdx::where(maskB, planeB) = 0x00'10'10'10;
    stdx::where(maskF, icfF) = 63;
    stdx::where(maskB, icfB) = 63;

    SIMD rfp = planeF >> 16 & 0xFF;
    SIMD gfp = planeF >> 8 & 0xFF;
    SIMD bfp = planeF & 0xFF;

    SIMD rbp = planeB >> 16 & 0xFF;
    SIMD gbp = planeB >> 8 & 0xFF;
    SIMD bbp = planeB & 0xFF;

    rfp -= 16;
    gfp -= 16;
    bfp -= 16;

    rbp -= 16;
    gbp -= 16;
    bbp -= 16;

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

    rfp += 16;
    gfp += 16;
    bfp += 16;
    // Don't add 16 to back plane when applying ICF because the below mixing subtracts it.

    rfp += rbp;
    gfp += gbp;
    bfp += bbp;

    rfp = stdx::clamp(rfp, U8_MIN, U8_MAX);
    gfp = stdx::clamp(gfp, U8_MIN, U8_MAX);
    bfp = stdx::clamp(bfp, U8_MIN, U8_MAX);

    const SIMD result = (rfp << 16) | (gfp << 8) | bfp;

    result.copy_to(screen->AsU32Pointer(), stdx::element_aligned);
}
// template void applyICFMixSIMDShift<false>() noexcept;
// template void applyICFMixSIMDShift<true>() noexcept;

/** \brief Applies ICF and mixes using SIMD (algorithm that casts the registers to access RGB components).
 * This can't be used with fixed-sized SIMD because fixed_sized_simd is not trivially copyable.
 */
static constexpr void applyICFMixSIMDCast(Pixel* screen, const Pixel* planeFront, const Pixel* planeBack, const uint8_t* icfFront, const uint8_t* icfBack) noexcept
{
    SIMDNativePixel icfF{icfFront, stdx::element_aligned};
    SIMDNativePixel icfB{icfBack, stdx::element_aligned};

    SIMDNativePixel planeF{planeFront->AsU32Pointer(), stdx::element_aligned};
    SIMDNativePixel planeB{planeBack->AsU32Pointer(), stdx::element_aligned};

    // transparent areas of an image simply give no contribution to the final display
    // - that is they are equivalent to black areas.
    const SIMDNativePixel::mask_type maskF = (planeF & ALPHA_MASKK) == 0;
    const SIMDNativePixel::mask_type maskB = (planeB & ALPHA_MASKK) == 0;
    stdx::where(maskF, planeF) = 0x00'10'10'10;
    stdx::where(maskB, planeB) = 0x00'10'10'10;
    stdx::where(maskF, icfF) = 63;
    stdx::where(maskB, icfB) = 63;

    // extend ICF to whole register.
    icfF *= 0x00'01'01'01;
    icfB *= 0x00'01'01'01;
    // icfF |= (icfF << 16) | (icfF << 8);
    // icfB |= (icfB << 16) | (icfB << 8);

    const SIMDNativeU8 rgbF8 = std::bit_cast<SIMDNativeU8>(planeF);
    const SIMDNativeU8 rgbB8 = std::bit_cast<SIMDNativeU8>(planeB);
    const SIMDNativeU8 icfF8 = std::bit_cast<SIMDNativeU8>(icfF);
    const SIMDNativeU8 icfB8 = std::bit_cast<SIMDNativeU8>(icfB);

    SIMDFixedS16 rgbF16 = stdx::static_simd_cast<int16_t>(rgbF8);
    SIMDFixedS16 rgbB16 = stdx::static_simd_cast<int16_t>(rgbB8);
    SIMDFixedS16 icfF16 = stdx::static_simd_cast<int16_t>(icfF8);
    SIMDFixedS16 icfB16 = stdx::static_simd_cast<int16_t>(icfB8);

    rgbF16 -= 16;
    rgbB16 -= 16;

    rgbF16 *= icfF16;
    rgbB16 *= icfB16;

    rgbF16 /= 63;
    rgbB16 /= 63;

    // rgbF16 >>= 6;
    // rgbB16 >>= 6;

    rgbF16 += 16;
    // rgbB16 += 16; Don't add 16 to back plane when applying ICF because the below mixing subtracts it.

    rgbF16 += rgbB16;

    rgbF16 = stdx::clamp(rgbF16, U8_MINN, U8_MAXX);

    const SIMDNativePixel result = std::bit_cast<SIMDNativePixel>(stdx::static_simd_cast<SIMDNativeU8>(rgbF16));

    result.copy_to(screen->AsU32Pointer(), stdx::element_aligned);
}
// template void applyICFMixSIMDCast<false>() noexcept;
// template void applyICFMixSIMDCast<true>() noexcept;

/** \brief Applies ICF and overlays using SIMD (algorithm that shifts and masks RGB components).
 * \tparam SIMD The SIMD type holding signed 32 bits integers.
 */
template<typename SIMD>
static constexpr void applyICFOverlaySIMDShift(Pixel* screen, const Pixel* planeFront, const Pixel* planeBack, const uint8_t* icfFront, const uint8_t* icfBack, const uint32_t backdrop) noexcept
{
    SIMD planeF{planeFront->AsU32Pointer(), stdx::element_aligned};
    SIMD planeB{planeBack->AsU32Pointer(), stdx::element_aligned};
    const SIMD icfF{icfFront, stdx::element_aligned};
    const SIMD icfB{icfBack, stdx::element_aligned};

    const SIMD afp = planeF >> 24 & 0xFF;
    SIMD rfp = planeF >> 16 & 0xFF;
    SIMD gfp = planeF >> 8 & 0xFF;
    SIMD bfp = planeF & 0xFF;

    const SIMD abp = planeB >> 24 & 0xFF;
    SIMD rbp = planeB >> 16 & 0xFF;
    SIMD gbp = planeB >> 8 & 0xFF;
    SIMD bbp = planeB & 0xFF;

    rfp -= 16;
    gfp -= 16;
    bfp -= 16;

    rbp -= 16;
    gbp -= 16;
    bbp -= 16;

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

    rfp += 16;
    gfp += 16;
    bfp += 16;

    rbp += 16;
    gbp += 16;
    bbp += 16;

    planeF = (rfp << 16) | (gfp << 8) | bfp;
    planeB = (rbp << 16) | (gbp << 8) | bbp;

    SIMD result{planeF};
    const typename SIMD::mask_type maskFrontZero = afp == 0;
    stdx::where(maskFrontZero, result) = planeB;
    const typename SIMD::mask_type maskFrontBackZero = maskFrontZero && abp == 0;
    stdx::where(maskFrontBackZero, result) = backdrop;

    result.copy_to(screen->AsU32Pointer(), stdx::element_aligned);
}

/** \brief Applies ICF and overlays using SIMD (algorithm that casts the registers to access RGB components).
 * This can't be used with fixed-sized SIMD because fixed_sized_simd is not trivially copyable.
 */
static constexpr void applyICFOverlaySIMDCast(Pixel* screen, const Pixel* planeFront, const Pixel* planeBack, const uint8_t* icfFront, const uint8_t* icfBack, const uint32_t backdrop) noexcept
{
    SIMDNativePixel icfF{icfFront, stdx::element_aligned};
    SIMDNativePixel icfB{icfBack, stdx::element_aligned};

    SIMDNativePixel planeF{planeFront->AsU32Pointer(), stdx::element_aligned};
    SIMDNativePixel planeB{planeBack->AsU32Pointer(), stdx::element_aligned};

    // transparent areas of an image simply give no contribution to the final display
    // - that is they are equivalent to black areas.
    const SIMDNativePixel::mask_type maskF = (planeF & ALPHA_MASKK) == 0;
    const SIMDNativePixel::mask_type maskB = (planeB & ALPHA_MASKK) == 0;
    stdx::where(maskF, planeF) = 0x00'10'10'10;
    stdx::where(maskB, planeB) = 0x00'10'10'10;
    stdx::where(maskF, icfF) = 63;
    stdx::where(maskB, icfB) = 63;

    // extend ICF to whole register.
    icfF *= 0x00'01'01'01;
    icfB *= 0x00'01'01'01;
    // icfF |= (icfF << 16) | (icfF << 8);
    // icfB |= (icfB << 16) | (icfB << 8);

    const SIMDNativeU8 rgbF8 = std::bit_cast<SIMDNativeU8>(planeF);
    const SIMDNativeU8 rgbB8 = std::bit_cast<SIMDNativeU8>(planeB);
    const SIMDNativeU8 icfF8 = std::bit_cast<SIMDNativeU8>(icfF);
    const SIMDNativeU8 icfB8 = std::bit_cast<SIMDNativeU8>(icfB);

    SIMDFixedS16 rgbF16 = stdx::static_simd_cast<int16_t>(rgbF8);
    SIMDFixedS16 rgbB16 = stdx::static_simd_cast<int16_t>(rgbB8);
    SIMDFixedS16 icfF16 = stdx::static_simd_cast<int16_t>(icfF8);
    SIMDFixedS16 icfB16 = stdx::static_simd_cast<int16_t>(icfB8);

    rgbF16 -= 16;
    rgbB16 -= 16;

    rgbF16 *= icfF16;
    rgbB16 *= icfB16;

    rgbF16 /= 63;
    rgbB16 /= 63;

    // rgbF16 >>= 6;
    // rgbB16 >>= 6;

    rgbF16 += 16;
    rgbB16 += 16;

    const SIMDNativePixel resultF{std::bit_cast<SIMDNativePixel>(stdx::static_simd_cast<SIMDNativeU8>(rgbF16))};
    const SIMDNativePixel resultB{std::bit_cast<SIMDNativePixel>(stdx::static_simd_cast<SIMDNativeU8>(rgbB16))};

    SIMDNativePixel result{resultF};
    stdx::where(maskF, result) = resultB;
    stdx::where(maskF && maskB, result) = backdrop;

    result.copy_to(screen->AsU32Pointer(), stdx::element_aligned);
}

/** \brief Dispatches the correct over or mix SIMD algorithm.
 * \tparam MIX true to use mixing, false to use overlay.
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 * \tparam WIDTH_REMINDER The width in pixels of the line module the native SIMD Pixel size.
 * Because this uses the same buffers as RendererSoftware, this algorithm makes sure we only read and write the
 * necessary amount of data (no more than the width of the screen and planes).
 * Because fixed-sized SIMD is not trivially copyable, Shift algorithm is used for the last loop with the reminder.
 */
template<bool MIX, bool PLANE_ORDER, size_t WIDTH_REMINDER>
void RendererSIMD::HandleOverlayMixSIMD() noexcept
{
    Pixel* screen = m_screen.GetLinePointer(m_lineNumber);
    const Pixel* planeFront;
    const Pixel* planeBack;
    const uint8_t* icfFront;
    const uint8_t* icfBack;
    if constexpr(PLANE_ORDER)
    {
        planeFront = m_plane[B].GetLinePointer(m_lineNumber);
        planeBack = m_plane[A].GetLinePointer(m_lineNumber);
        icfFront = m_icfLine[B].data();
        icfBack = m_icfLine[A].data();
    }
    else
    {
        planeFront = m_plane[A].GetLinePointer(m_lineNumber);
        planeBack = m_plane[B].GetLinePointer(m_lineNumber);
        icfFront = m_icfLine[A].data();
        icfBack = m_icfLine[B].data();
    }

    // Both planes always have the same width.
    constexpr int simdSize = static_cast<int>(SIMD_SIZE);
    for(int width = static_cast<int>(m_plane[A].m_width); width >= simdSize; width -= simdSize,
        planeFront += SIMD_SIZE, planeBack += SIMD_SIZE, icfFront += SIMD_SIZE, icfBack += SIMD_SIZE, screen += SIMD_SIZE)
    {
        if constexpr(MIX) // Mixing.
            applyICFMixSIMDCast(screen, planeFront, planeBack, icfFront, icfBack);
            // applyICFMixSIMDShift<SIMDNativePixelSigned>(screen, planeFront, planeBack, icfFront, icfBack);
        else // Overlay.
            applyICFOverlaySIMDCast(screen, planeFront, planeBack, icfFront, icfBack, m_backdropPlane.GetLinePointer(m_lineNumber)->AsU32());
            // applyICFOverlaySIMDShift<SIMDNativePixelSigned>(screen, planeFront, planeBack, icfFront, icfBack, m_backdropPlane.GetLinePointer(m_lineNumber)->AsU32());
    }

    if constexpr(WIDTH_REMINDER != 0) // Not a multiple of SIMD width.
    {
        // Now the remaining width is less than a SIMD register.
        if constexpr(MIX)
            applyICFMixSIMDShift<SIMDFixedPixelSigned<WIDTH_REMINDER>>(screen, planeFront, planeBack, icfFront, icfBack);
        else
            applyICFOverlaySIMDShift<SIMDFixedPixelSigned<WIDTH_REMINDER>>(screen, planeFront, planeBack, icfFront, icfBack, m_backdropPlane.GetLinePointer(m_lineNumber)->AsU32());
    }
}

} // namespace Video
