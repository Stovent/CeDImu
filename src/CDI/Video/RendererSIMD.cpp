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
        const uint16_t width = getDisplayWidth(m_displayFormat);
        const uint16_t height = GetDisplayHeight();

        m_screen.m_width = m_plane[A].m_width = m_plane[B].m_width = width * 2;
        m_screen.m_height = m_plane[A].m_height = m_plane[B].m_height = m_backdropPlane.m_height = height;
    }

    ResetMatte();

    uint16_t bytesA = DrawLinePlane<A>(lineA, nullptr); // nullptr because plane A can't decode RGB555.
    const uint16_t bytesB = DrawLinePlane<B>(lineB, lineA);
    if(m_codingMethod[B] == ImageCodingMethod::RGB555)
        bytesA = bytesB;

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
template<ImagePlane PLANE>
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
    // is outputted continuously line by line). But for here maybe we don't care.
    const Pixel color = GetCursorColor();

    using SIMDCursorLine = stdx::fixed_size_simd<uint32_t, 16>;
    using SIMDCursorLineMask = SIMDCursorLine::mask_type;
    static constexpr SIMDCursorLine PATTERN_MASK([] (uint32_t i) { return 1 << (15 - i); });

    const SIMDCursorLine blackSimd{BLACK_PIXEL.AsU32()};
    const SIMDCursorLine colorSimd{color.AsU32()};
    int patternIndex = 0;
    for(Plane::iterator dst = m_cursorPlane.begin(); dst < m_cursorPlane.end(); dst += SIMDCursorLine::size(), ++patternIndex)
    {
        // Convert pattern to a mask.
        const SIMDCursorLine patternSimd = m_cursorPatterns[patternIndex];
        const SIMDCursorLineMask patternMask = (patternSimd & PATTERN_MASK) != 0;

        SIMDCursorLine cursorPixels = blackSimd;
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
    if(m_matteNumber)
        HandleMatteSIMD<true>();
    else
        HandleMatteSIMD<false>();

    switch(m_screen.m_width)
    {
    case 720:
        HandleTransparencyPlaneASIMD<SIMDReminder<720>::value>();
        HandleTransparencyPlaneBSIMD<SIMDReminder<720>::value>();
        HandleOverlayMixSIMD<MIX, PLANE_ORDER, SIMDReminder<720>::value>();
        break;

    case 768:
        HandleTransparencyPlaneASIMD<SIMDReminder<768>::value>();
        HandleTransparencyPlaneBSIMD<SIMDReminder<768>::value>();
        HandleOverlayMixSIMD<MIX, PLANE_ORDER, SIMDReminder<768>::value>();
        break;

    default:
        std::unreachable();
    }
}

template<size_t WIDTH>
inline constexpr SIMDFixedPixelSigned<WIDTH> U8_MIN{0};
template<size_t WIDTH>
inline constexpr SIMDFixedPixelSigned<WIDTH> U8_MAX{255};
template<size_t WIDTH>
inline constexpr SIMDFixedPixelSigned<WIDTH> ALPHA_MASK{-16777216}; // 0xFF'00'00'00

inline constexpr SIMDFixedS16 U8_MINN{0};
inline constexpr SIMDFixedS16 U8_MAXX{255};
inline constexpr SIMDNativePixel ALPHA_MASKK{0xFF'00'00'00};

/** \brief Applies ICF and mixes using SIMD (algorithm that shifts and masks RGB components).
 * \tparam WIDTH The width of the SIMD type.
 */
template<size_t WIDTH>
static constexpr void applyICFMixSIMDShift(Pixel* screen, const Pixel* planeFront, const Pixel* planeBack, const uint8_t* icfFront, const uint8_t* icfBack, const uint32_t backdrop) noexcept
{
    using SIMD = SIMDFixedPixelSigned<WIDTH>;

    SIMD icfF{icfFront, stdx::element_aligned};
    SIMD icfB{icfBack, stdx::element_aligned};

    SIMD planeF{planeFront->AsU32Pointer(), stdx::element_aligned};
    SIMD planeB{planeBack->AsU32Pointer(), stdx::element_aligned};

    // transparent areas of an image simply give no contribution to the final display
    // - that is they are equivalent to black areas.
    const typename SIMD::mask_type transparentF = (planeF & ALPHA_MASK<WIDTH>) == 0;
    const typename SIMD::mask_type transparentB = (planeB & ALPHA_MASK<WIDTH>) == 0;
    stdx::where(transparentF, planeF) = 0x00'10'10'10;
    stdx::where(transparentB, planeB) = 0x00'10'10'10;
    stdx::where(transparentF, icfF) = 63;
    stdx::where(transparentB, icfB) = 63;

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
    // Don't add 16 to back plane when applying ICF because mixing subtracts it.

    rfp += rbp;
    gfp += gbp;
    bfp += bbp;

    rfp = stdx::clamp(rfp, U8_MIN<WIDTH>, U8_MAX<WIDTH>);
    gfp = stdx::clamp(gfp, U8_MIN<WIDTH>, U8_MAX<WIDTH>);
    bfp = stdx::clamp(bfp, U8_MIN<WIDTH>, U8_MAX<WIDTH>);

    SIMD result = (rfp << 16) | (gfp << 8) | bfp;
    stdx::where(transparentF && transparentB, result) = backdrop;
    result |= ALPHA_MASK<WIDTH>; // Screen is always visible.

    result.copy_to(screen->AsU32Pointer(), stdx::element_aligned);
}
// template void applyICFMixSIMDShift<8>(Pixel* screen, const Pixel* planeFront, const Pixel* planeBack, const uint8_t* icfFront, const uint8_t* icfBack, const uint32_t backdrop) noexcept;

/** \brief Applies ICF and mixes using SIMD (algorithm that casts the registers to access RGB components).
 * This can't be used with fixed-sized SIMD because fixed_sized_simd is not trivially copyable.
 */
static constexpr void applyICFMixSIMDCast(Pixel* screen, const Pixel* planeFront, const Pixel* planeBack, const uint8_t* icfFront, const uint8_t* icfBack, const uint32_t backdrop) noexcept
{
    SIMDNativePixel icfF{icfFront, stdx::element_aligned};
    SIMDNativePixel icfB{icfBack, stdx::element_aligned};

    SIMDNativePixel planeF{planeFront->AsU32Pointer(), stdx::element_aligned};
    SIMDNativePixel planeB{planeBack->AsU32Pointer(), stdx::element_aligned};

    // transparent areas of an image simply give no contribution to the final display
    // - that is they are equivalent to black areas.
    const SIMDNativePixel::mask_type transparentF = (planeF & ALPHA_MASKK) == 0;
    const SIMDNativePixel::mask_type transparentB = (planeB & ALPHA_MASKK) == 0;
    stdx::where(transparentF, planeF) = 0x00'10'10'10;
    stdx::where(transparentB, planeB) = 0x00'10'10'10;
    stdx::where(transparentF, icfF) = 63;
    stdx::where(transparentB, icfB) = 63;

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
    // Don't add 16 to back plane when applying ICF because mixing subtracts it.

    rgbF16 += rgbB16;

    rgbF16 = stdx::clamp(rgbF16, U8_MINN, U8_MAXX);

    SIMDNativePixel result = std::bit_cast<SIMDNativePixel>(stdx::static_simd_cast<SIMDNativeU8>(rgbF16));
    stdx::where(transparentF && transparentB, result) = backdrop;
    result |= ALPHA_MASKK; // Screen is always visible.

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

    rbp += 16;
    gbp += 16;
    bbp += 16;

    // Keep alpha channel.
    planeF &= 0xFF'00'00'00;
    planeB &= 0xFF'00'00'00;
    const typename SIMD::mask_type maskFrontZero = planeF == 0;
    const typename SIMD::mask_type maskFrontBackZero = maskFrontZero && planeB == 0;

    planeF |= (rfp << 16) | (gfp << 8) | bfp;
    planeB |= (rbp << 16) | (gbp << 8) | bbp;

    SIMD result{planeF};
    stdx::where(maskFrontZero, result) = planeB;
    stdx::where(maskFrontBackZero, result) = backdrop;

    result.copy_to(screen->AsU32Pointer(), stdx::element_aligned);
}

/** \brief Applies ICF and overlays using SIMD (algorithm that casts the registers to access RGB components).
 * This can't be used with fixed-sized SIMD because fixed_sized_simd is not trivially copyable.
 */
static constexpr void applyICFOverlaySIMDCast(Pixel* screen, const Pixel* planeFront, const Pixel* planeBack, const uint8_t* icfFront, const uint8_t* icfBack, const uint32_t backdrop) noexcept
{
    SIMDNativePixel planeF{planeFront->AsU32Pointer(), stdx::element_aligned};
    SIMDNativePixel planeB{planeBack->AsU32Pointer(), stdx::element_aligned};

    const SIMDNativePixel::mask_type transparentF = (planeF & ALPHA_MASKK) == 0;
    const SIMDNativePixel::mask_type transparentB = (planeB & ALPHA_MASKK) == 0;
    // Transparent pixels are overwritten by visible pixels in the end so no need to adjust ICF and black level.

    SIMDNativePixel icfF{icfFront, stdx::element_aligned};
    SIMDNativePixel icfB{icfBack, stdx::element_aligned};

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
    stdx::where(transparentF, result) = resultB;
    stdx::where(transparentF && transparentB, result) = backdrop;
    result |= ALPHA_MASKK; // Screen is always visible.

    result.copy_to(screen->AsU32Pointer(), stdx::element_aligned);
}

/** \brief Dispatches the correct overlay or mix SIMD algorithm.
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

    for(size_t width = m_screen.m_width; width >= SIMD_SIZE; width -= SIMD_SIZE,
        planeFront += SIMD_SIZE, planeBack += SIMD_SIZE, icfFront += SIMD_SIZE, icfBack += SIMD_SIZE, screen += SIMD_SIZE)
    {
        if constexpr(MIX) // Mixing.
            applyICFMixSIMDCast(screen, planeFront, planeBack, icfFront, icfBack, m_backdropPlane.GetLinePointer(m_lineNumber)->AsU32());
        else // Overlay.
            applyICFOverlaySIMDCast(screen, planeFront, planeBack, icfFront, icfBack, m_backdropPlane.GetLinePointer(m_lineNumber)->AsU32());
    }

    if constexpr(WIDTH_REMINDER != 0) // Now the remaining width is less than a SIMD register.
    {
        if constexpr(MIX)
            applyICFMixSIMDShift<WIDTH_REMINDER>(screen, planeFront, planeBack, icfFront, icfBack, m_backdropPlane.GetLinePointer(m_lineNumber)->AsU32());
        else
            applyICFOverlaySIMDShift<SIMDFixedPixelSigned<WIDTH_REMINDER>>(screen, planeFront, planeBack, icfFront, icfBack, m_backdropPlane.GetLinePointer(m_lineNumber)->AsU32());
    }
}

/** \brief Executes the given matte command.
 * \tparam TWO_MATTES true for two mattes, false for one matte.
 * \param command The command to execute.
 * \param mf The matte flag to modifiy (used only when TWO_MATTES is true).
 * \return true if upper registers are to be ignored (command 0).
 */
template<bool TWO_MATTES>
bool RendererSIMD::ExecuteMatteCommand(const uint32_t command, bool mf) noexcept
{
    if constexpr(!TWO_MATTES)
        mf = matteMF(command);

    const uint8_t op = matteOp(command);
    switch(op)
    {
    case 0b0000:
        return true;

    case 0b0100:
        m_icf[A] = matteICF(command);
        break;

    case 0b0110:
        m_icf[B] = matteICF(command);
        break;

    case 0b1000:
        m_matteFlags[mf] = false;
        break;

    case 0b1001:
        m_matteFlags[mf] = true;
        break;

    case 0b1100:
        m_icf[A] = matteICF(command);
        m_matteFlags[mf] = false;
        break;

    case 0b1101:
        m_icf[A] = matteICF(command);
        m_matteFlags[mf] = true;
        break;

    case 0b1110:
        m_icf[B] = matteICF(command);
        m_matteFlags[mf] = false;
        break;

    case 0b1111:
        m_icf[B] = matteICF(command);
        m_matteFlags[mf] = true;
        break;
    }

    return false;
}

template<bool TWO_MATTES>
void RendererSIMD::HandleMatteSIMD() noexcept
{
    // No need to reset m_matteFlagsLine to false.

    size_t nextMatte0 = 0; // Used when 1 or 2 mattes.
    size_t nextMatte1 = MATTE_HALF; // Used when 2 mattes.
    size_t nextChange0 = matteXPosition(m_matteControl[nextMatte0]);
    size_t nextChange1 = matteXPosition(m_matteControl[nextMatte1]);

    for(size_t x = 0; x < m_screen.m_width;)
    {
        size_t nextChange = m_screen.m_width;

        if constexpr(TWO_MATTES)
        {
            if(nextChange0 == x)
            {
                const uint32_t command0 = m_matteControl[nextMatte0];
                const bool disregard = ExecuteMatteCommand<TWO_MATTES>(command0, false); // false for matte 0.

                ++nextMatte0;
                if(disregard || nextMatte0 >= MATTE_HALF)
                    nextChange0 = m_screen.m_width;
                else
                    nextChange0 = matteXPosition(m_matteControl[nextMatte0]);
            }

            if(nextChange1 == x)
            {
                const uint32_t command1 = m_matteControl[nextMatte1];
                const bool disregard = ExecuteMatteCommand<TWO_MATTES>(command1, true); // true for matte 1.

                ++nextMatte1;
                if(disregard || nextMatte1 >= MATTE_NUM)
                    nextChange1 = m_screen.m_width;
                else
                    nextChange1 = matteXPosition(m_matteControl[nextMatte1]);
            }

            // Sometimes the next register has a lower position.
            if(nextChange0 <= x)
                nextChange0 = m_screen.m_width;
            if(nextChange1 <= x)
                nextChange1 = m_screen.m_width;
            nextChange = std::min(nextChange0, nextChange1);
        }
        else
        {
            if(nextChange0 == x)
            {
                const uint32_t command = m_matteControl[nextMatte0];
                const bool disregard = ExecuteMatteCommand<TWO_MATTES>(command, false); // false is unused with one matte.

                ++nextMatte0;
                if(disregard || nextMatte0 >= m_matteControl.size())
                    nextChange0 = m_screen.m_width;
                else
                    nextChange0 = matteXPosition(m_matteControl[nextMatte0]);
            }
            nextChange = nextChange0;
        }

        for(; x < nextChange; ++x)
        {
            m_icfLine[A][x] = m_icf[A];
            m_icfLine[B][x] = m_icf[B];
            m_matteFlagsLine[A][x] = m_matteFlags[A];
            m_matteFlagsLine[B][x] = m_matteFlags[B];
        }
    }
}

/** \brief Dispatch transparency of plane A. */
template<size_t WIDTH_REM>
void RendererSIMD::HandleTransparencyPlaneASIMD() noexcept
{
    const bool booleanA = !bit<3>(m_transparencyControl[A]);
    const uint8_t controlA = bits<0, 2>(m_transparencyControl[A]);

    switch(static_cast<TransparentIf>(controlA))
    {
    case TransparentIf::AlwaysNever: // Always/Never.
        if(booleanA)
            HandleTransparencyLoopSIMD<WIDTH_REM, A, TransparentIf::AlwaysNever, true>();
        else
            HandleTransparencyLoopSIMD<WIDTH_REM, A, TransparentIf::AlwaysNever, false>();
        break;

    case TransparentIf::ColorKey: // Color Key.
        if(booleanA)
            HandleTransparencyLoopSIMD<WIDTH_REM, A, TransparentIf::ColorKey, true>();
        else
            HandleTransparencyLoopSIMD<WIDTH_REM, A, TransparentIf::ColorKey, false>();
        break;

    case TransparentIf::TransparencyBit: // Transparent Bit.
        // TODO: currently decodeRGB555 make the pixel visible if the bit is set.
        // TODO: disable if not RGB555.
        if(booleanA)
            HandleTransparencyLoopSIMD<WIDTH_REM, A, TransparentIf::TransparencyBit, true>();
        else
            HandleTransparencyLoopSIMD<WIDTH_REM, A, TransparentIf::TransparencyBit, false>();
        break;

    case TransparentIf::MatteFlag0: // Matte Flag 0.
        if(booleanA)
            HandleTransparencyLoopSIMD<WIDTH_REM, A, TransparentIf::MatteFlag0, true>();
        else
            HandleTransparencyLoopSIMD<WIDTH_REM, A, TransparentIf::MatteFlag0, false>();
        break;

    case TransparentIf::MatteFlag1: // Matte Flag 1.
        if(booleanA)
            HandleTransparencyLoopSIMD<WIDTH_REM, A, TransparentIf::MatteFlag1, true>();
        else
            HandleTransparencyLoopSIMD<WIDTH_REM, A, TransparentIf::MatteFlag1, false>();
        break;

    case TransparentIf::MatteFlag0OrColorKey: // Matte Flag 0 or Color Key.
        if(booleanA)
            HandleTransparencyLoopSIMD<WIDTH_REM, A, TransparentIf::MatteFlag0OrColorKey, true>();
        else
            HandleTransparencyLoopSIMD<WIDTH_REM, A, TransparentIf::MatteFlag0OrColorKey, false>();
        break;

    case TransparentIf::MatteFlag1OrColorKey: // Matte Flag 1 or Color Key.
        if(booleanA)
            HandleTransparencyLoopSIMD<WIDTH_REM, A, TransparentIf::MatteFlag1OrColorKey, true>();
        else
            HandleTransparencyLoopSIMD<WIDTH_REM, A, TransparentIf::MatteFlag1OrColorKey, false>();
        break;

    default: // Reserved.
        std::unreachable();
        break;
    }
}

/** \brief Dispatch transparency of plane B. */
template<size_t WIDTH_REM>
void RendererSIMD::HandleTransparencyPlaneBSIMD() noexcept
{
    const bool booleanB = !bit<3>(m_transparencyControl[B]);
    const uint8_t controlB = bits<0, 2>(m_transparencyControl[B]);

    switch(static_cast<TransparentIf>(controlB))
    {
    case TransparentIf::AlwaysNever: // Always/Never.
        if(booleanB)
            HandleTransparencyLoopSIMD<WIDTH_REM, B, TransparentIf::AlwaysNever, true>();
        else
            HandleTransparencyLoopSIMD<WIDTH_REM, B, TransparentIf::AlwaysNever, false>();
        break;

    case TransparentIf::ColorKey: // Color Key.
        if(booleanB)
            HandleTransparencyLoopSIMD<WIDTH_REM, B, TransparentIf::ColorKey, true>();
        else
            HandleTransparencyLoopSIMD<WIDTH_REM, B, TransparentIf::ColorKey, false>();
        break;

    case TransparentIf::TransparencyBit: // Transparent Bit.
        // TODO: currently decodeRGB555 make the pixel visible if the bit is set.
        // TODO: disable if not RGB555.
        if(booleanB)
            HandleTransparencyLoopSIMD<WIDTH_REM, B, TransparentIf::TransparencyBit, true>();
        else
            HandleTransparencyLoopSIMD<WIDTH_REM, B, TransparentIf::TransparencyBit, false>();
        break;

    case TransparentIf::MatteFlag0: // Matte Flag 0.
        if(booleanB)
            HandleTransparencyLoopSIMD<WIDTH_REM, B, TransparentIf::MatteFlag0, true>();
        else
            HandleTransparencyLoopSIMD<WIDTH_REM, B, TransparentIf::MatteFlag0, false>();
        break;

    case TransparentIf::MatteFlag1: // Matte Flag 1.
        if(booleanB)
            HandleTransparencyLoopSIMD<WIDTH_REM, B, TransparentIf::MatteFlag1, true>();
        else
            HandleTransparencyLoopSIMD<WIDTH_REM, B, TransparentIf::MatteFlag1, false>();
        break;

    case TransparentIf::MatteFlag0OrColorKey: // Matte Flag 0 or Color Key.
        if(booleanB)
            HandleTransparencyLoopSIMD<WIDTH_REM, B, TransparentIf::MatteFlag0OrColorKey, true>();
        else
            HandleTransparencyLoopSIMD<WIDTH_REM, B, TransparentIf::MatteFlag0OrColorKey, false>();
        break;

    case TransparentIf::MatteFlag1OrColorKey: // Matte Flag 1 or Color Key.
        if(booleanB)
            HandleTransparencyLoopSIMD<WIDTH_REM, B, TransparentIf::MatteFlag1OrColorKey, true>();
        else
            HandleTransparencyLoopSIMD<WIDTH_REM, B, TransparentIf::MatteFlag1OrColorKey, false>();
        break;

    default: // Reserved.
        std::unreachable();
        break;
    }
}

static constexpr Pixel::ARGB32 COLOR_KEY_MASK = 0x00'FC'FC'FC;

template<Renderer::TransparentIf TRANSPARENT, bool BOOL_FLAG, typename SIMD>
constexpr void HandleTransparencySIMD(Pixel* plane, const bool* matteFlagsA, const bool* matteFlagsB, SIMD colorMask, SIMD transparentColor) noexcept;

/** \brief Actually handles the transparency for a plane statically. */
template<size_t WIDTH_REM, ImagePlane PLANE, Renderer::TransparentIf TRANSPARENT, bool BOOL_FLAG>
void RendererSIMD::HandleTransparencyLoopSIMD() noexcept
{
    const SIMDNativePixel colorMask{m_maskColorRgb[PLANE] & COLOR_KEY_MASK};
    const SIMDNativePixel transparentColor{(m_transparentColorRgb[PLANE] & COLOR_KEY_MASK) | colorMask};

    Pixel* plane = m_plane[PLANE].GetLinePointer(m_lineNumber);
    size_t remaining = m_plane[PLANE].m_width;
    size_t i = 0;
    for(; remaining >= SIMDNativePixel::size();
        remaining -= SIMDNativePixel::size(), i += SIMDNativePixel::size(), plane += SIMDNativePixel::size())
    {
        const bool* matteFlagsA = m_matteFlagsLine[A].data() + i;
        const bool* matteFlagsB = m_matteFlagsLine[B].data() + i;
        HandleTransparencySIMD<TRANSPARENT, BOOL_FLAG, SIMDNativePixel>(plane, matteFlagsA, matteFlagsB, colorMask, transparentColor);
    }

    if constexpr(WIDTH_REM != 0)
    {
        const SIMDFixedPixel<WIDTH_REM> colorMaskFixed{m_maskColorRgb[PLANE] & COLOR_KEY_MASK};
        const SIMDFixedPixel<WIDTH_REM> transparentColorFixed{(m_transparentColorRgb[PLANE] & COLOR_KEY_MASK) | colorMaskFixed};
        const bool* matteFlagsA = m_matteFlagsLine[A].data() + i;
        const bool* matteFlagsB = m_matteFlagsLine[B].data() + i;
        HandleTransparencySIMD<TRANSPARENT, BOOL_FLAG, SIMDFixedPixel<WIDTH_REM>>(plane, matteFlagsA, matteFlagsB, colorMaskFixed, transparentColorFixed);
    }
}
template void RendererSIMD::HandleTransparencyLoopSIMD<8, A, Renderer::TransparentIf::AlwaysNever, false>() noexcept;

template<Renderer::TransparentIf TRANSPARENT, bool BOOL_FLAG, typename SIMD>
constexpr void HandleTransparencySIMD(Pixel* plane, const bool* matteFlagsA, const bool* matteFlagsB, SIMD colorMask, SIMD transparentColor) noexcept
{
    using MASK = SIMD::mask_type;
    constexpr MASK FLAG{BOOL_FLAG};
    constexpr SIMD SET_ALPHA{0xFF'00'00'00};
    constexpr SIMD CLEAR_ALPHA{0x00'FF'FF'FF};

    SIMD pixel{plane, stdx::element_aligned};
    pixel |= 0xFF'00'00'00; // Set to visible.

    const MASK colorKey = ((pixel & SIMD{COLOR_KEY_MASK}) | colorMask) == transparentColor;

    switch(TRANSPARENT)
    {
    case Renderer::TransparentIf::AlwaysNever: // Always/Never.
        stdx::where(FLAG, pixel) &= CLEAR_ALPHA;
        break;

    case Renderer::TransparentIf::ColorKey: // Color Key.
        stdx::where(colorKey == FLAG, pixel) &= CLEAR_ALPHA;
        break;

    case Renderer::TransparentIf::TransparencyBit: // Transparent Bit.
    {
        // TODO: currently decodeRGB555 make the pixel visible if the bit is set.
        // TODO: disable if not RGB555.
        const MASK mask = ((pixel & SET_ALPHA) != 0) == FLAG;
        stdx::where(mask, pixel) &= CLEAR_ALPHA;
        break;
    }

    case Renderer::TransparentIf::MatteFlag0: // Matte Flag 0.
    {
        const MASK matte{matteFlagsA, stdx::element_aligned};
        stdx::where(matte == FLAG, pixel) &= CLEAR_ALPHA;
        break;
    }

    case Renderer::TransparentIf::MatteFlag1: // Matte Flag 1.
    {
        const MASK matte{matteFlagsB, stdx::element_aligned};
        stdx::where(matte == FLAG, pixel) &= CLEAR_ALPHA;
        break;
    }

    case Renderer::TransparentIf::MatteFlag0OrColorKey: // Matte Flag 0 or Color Key.
    {
        const MASK matte{matteFlagsA, stdx::element_aligned};
        stdx::where(matte == FLAG || colorKey == FLAG, pixel) &= CLEAR_ALPHA;
        break;
    }

    case Renderer::TransparentIf::MatteFlag1OrColorKey: // Matte Flag 1 or Color Key.
    {
        const MASK matte{matteFlagsB, stdx::element_aligned};
        stdx::where(matte == FLAG || colorKey == FLAG, pixel) &= CLEAR_ALPHA;
        break;
    }

    default: // Reserved.
        std::unreachable();
        break;
    }

    pixel.copy_to(plane->AsU32Pointer(), stdx::element_aligned);
}

} // namespace Video
