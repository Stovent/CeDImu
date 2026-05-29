/** \file RendererSIMDstd.cpp
 * \brief Implementation of RendererSIMD class with std::simd.
 *
 * The Cast version of Overlay/Mix should be faster than Shift, but previous benchmarks showed that Cast was slower on
 * AVX2, but I can't reproduce it now. So Cast is used for now.
 *
 * All these functions does not seem to benefit from factorizing the applyICF to a dedicated function, as MixSIMDCast
 * uses the intermediate representation for the computation, and OverlaySIMDCast reuses the mask from ICF computation.
 */

using SIMDCursorLine = std::simd::vec<uint32_t, 16>;
using SIMDCursorLineMask = SIMDCursorLine::mask_type;
static constexpr size_t SIMDCursorLineSize = SIMDCursorLine::size();

static constexpr std::array<uint32_t, 16> PATTERN_MASK{
    0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100,
    0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1,
};
static constexpr SIMDCursorLine PATTERN_MASK_SIMD{PATTERN_MASK, std::simd::flag_aligned};

void RendererSIMD::DrawCursor() noexcept
{
    // Technically speaking the cursor is drawn when the drawing line number is the cursor's one (because video
    // is outputted continuously line by line). But for here maybe we don't care.
    const Pixel color = GetCursorColor();
    const SIMDCursorLine colorSimd{color.AsU32()};
    const SIMDCursorLine blackSimd{BLACK_PIXEL.AsU32()};

    auto dst = m_cursorPlane.begin();
    for(const uint16_t pattern : m_cursorPatterns)
    {
        // Is there a way to very efficiently use the pattern variable directly and reverse its bits?
        // const SIMDCursorLineMask patternMask{pattern}; // Can std::simd swap all the elements efficiently?
        const SIMDCursorLine patternSimd{pattern};
        const SIMDCursorLineMask patternMask = (patternSimd & PATTERN_MASK_SIMD) != 0u;

        const SIMDCursorLine cursorPixels = std::simd::select(patternMask, colorSimd, blackSimd);
        std::simd::unchecked_store(cursorPixels, dst->AsU32Pointer(), SIMDCursorLineSize, std::simd::flag_aligned);

        dst += SIMDCursorLineSize;
    }
}

inline constexpr SIMDFixedS16 U8_MIN{static_cast<int16_t>(0)};
inline constexpr SIMDFixedS16 U8_MAX{static_cast<int16_t>(255)};
inline constexpr SIMDNativePixel ALPHA_MASK{0xFF'00'00'00u};

inline constexpr SIMDFixedS16 SIXTEEN{static_cast<int16_t>(16)};
inline constexpr SIMDFixedS16 SIXTYTHREE{static_cast<int16_t>(63)};

/** \brief Applies ICF and overlays/mix using SIMD (algorithm that casts the registers to access RGB components).
 */
template<bool MIX>
static constexpr void applyICFOverlayMixSIMDCast(Pixel* screen, const Pixel* planeFront, const Pixel* planeBack, const uint8_t* icfFront, const uint8_t* icfBack, const uint32_t backdrop) noexcept
{
    SIMDNativePixel planeF = std::simd::unchecked_load(planeFront->AsU32Pointer(), SIMDNativePixelSize, std::simd::flag_aligned);
    SIMDNativePixel planeB = std::simd::unchecked_load(planeBack->AsU32Pointer(), SIMDNativePixelSize, std::simd::flag_aligned);

    const SIMDNativePixelMask transparentF = (planeF & ALPHA_MASK) == 0u;
    const SIMDNativePixelMask transparentB = (planeB & ALPHA_MASK) == 0u;
    // Transparent pixels are overwritten by visible pixels in the end so no need to adjust ICF and black level.

    SIMDNativePixel icfF = std::simd::unchecked_load<SIMDNativePixel>(icfFront, SIMDNativePixelSize, std::simd::flag_aligned);
    SIMDNativePixel icfB = std::simd::unchecked_load<SIMDNativePixel>(icfBack, SIMDNativePixelSize, std::simd::flag_aligned);

    if constexpr(MIX)
    {
        planeF = std::simd::select(transparentF, 0x00'10'10'10u, planeF);
        planeB = std::simd::select(transparentB, 0x00'10'10'10u, planeB);
        icfF = std::simd::select(transparentF, 63u, icfF);
        icfB = std::simd::select(transparentB, 63u, icfB);
    }

    // extend ICF to whole register.
    icfF *= 0x00'01'01'01u;
    icfB *= 0x00'01'01'01u;
    // icfF |= (icfF << 16) | (icfF << 8);
    // icfB |= (icfB << 16) | (icfB << 8);

    const SIMDNativeU8 rgbF8 = std::bit_cast<SIMDNativeU8>(planeF);
    const SIMDNativeU8 rgbB8 = std::bit_cast<SIMDNativeU8>(planeB);
    const SIMDNativeU8 icfF8 = std::bit_cast<SIMDNativeU8>(icfF);
    const SIMDNativeU8 icfB8 = std::bit_cast<SIMDNativeU8>(icfB);

    SIMDFixedS16 rgbF16 = rgbF8;
    SIMDFixedS16 rgbB16 = rgbB8;
    SIMDFixedS16 icfF16 = icfF8;
    SIMDFixedS16 icfB16 = icfB8;

    rgbF16 -= SIXTEEN;
    rgbB16 -= SIXTEEN;

    rgbF16 *= icfF16;
    rgbB16 *= icfB16;

    rgbF16 /= SIXTYTHREE;
    rgbB16 /= SIXTYTHREE;

    // rgbF16 >>= 6;
    // rgbB16 >>= 6;

    rgbF16 += SIXTEEN;
    rgbB16 += SIXTEEN;

    const SIMDNativePixel resultF{std::bit_cast<SIMDNativePixel>(SIMDNativeU8(rgbF16))};
    const SIMDNativePixel resultB{std::bit_cast<SIMDNativePixel>(SIMDNativeU8(rgbB16))};

    SIMDNativePixel result;
    if constexpr(MIX)
    {
        // We subtract 16 because we may need to show only front or back, so they need their correct colors.
        const SIMDFixedS16 mixed16 = std::simd::clamp(rgbF16 + rgbB16 - SIXTEEN, U8_MIN, U8_MAX);
        result = std::bit_cast<SIMDNativePixel>(SIMDNativeU8(mixed16));
        const SIMDNativePixel mixed = std::bit_cast<SIMDNativePixel>(SIMDNativeU8(mixed16));

        result = std::simd::select(transparentF, backdrop, resultF);
        result = std::simd::select(transparentB, result, resultB);
        result = std::simd::select(!transparentF && !transparentB, mixed, result);
    }
    else
    {
        result = std::simd::select(transparentB, backdrop, resultB);
        result = std::simd::select(transparentF, result, resultF);
    }
    result |= ALPHA_MASK; // Screen is always visible.

    std::simd::unchecked_store(result, screen->AsU32Pointer(), SIMDNativePixelSize, std::simd::flag_aligned);
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
        applyICFOverlayMixSIMDCast<MIX>(screen, planeFront, planeBack, icfFront, icfBack, m_backdropPlane.GetLinePointer(m_lineNumber)->AsU32());
    }

    // if constexpr(WIDTH_REMINDER != 0) // Now the remaining width is less than a SIMD register.
    // {
    //     if constexpr(MIX)
    //         applyICFMixSIMDShift<WIDTH_REMINDER>(screen, planeFront, planeBack, icfFront, icfBack, m_backdropPlane.GetLinePointer(m_lineNumber)->AsU32());
    //     else
    //         applyICFOverlaySIMDShift<SIMDFixedPixelSigned<WIDTH_REMINDER>>(screen, planeFront, planeBack, icfFront, icfBack, m_backdropPlane.GetLinePointer(m_lineNumber)->AsU32());
    // }
}

template<Renderer::TransparentIf TRANSPARENT, bool BOOL_FLAG, typename SIMD>
static constexpr void HandleTransparencySIMD(Pixel* plane, const bool* matteFlagsA, const bool* matteFlagsB, SIMD colorMask, SIMD transparentColor) noexcept;

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

    // if constexpr(WIDTH_REM != 0)
    // {
    //     const SIMDFixedPixel<WIDTH_REM> colorMaskFixed{m_maskColorRgb[PLANE] & COLOR_KEY_MASK};
    //     const SIMDFixedPixel<WIDTH_REM> transparentColorFixed{(m_transparentColorRgb[PLANE] & COLOR_KEY_MASK) | colorMaskFixed};
    //     const bool* matteFlagsA = m_matteFlagsLine[A].data() + i;
    //     const bool* matteFlagsB = m_matteFlagsLine[B].data() + i;
    //     HandleTransparencySIMD<TRANSPARENT, BOOL_FLAG, SIMDFixedPixel<WIDTH_REM>>(plane, matteFlagsA, matteFlagsB, colorMaskFixed, transparentColorFixed);
    // }
}
template void RendererSIMD::HandleTransparencyLoopSIMD<8, A, Renderer::TransparentIf::AlwaysNever, false>() noexcept;

template<Renderer::TransparentIf TRANSPARENT, bool BOOL_FLAG, typename SIMD>
[[gnu::always_inline]]
static constexpr void HandleTransparencySIMD(Pixel* plane, const bool* matteFlagsA, const bool* matteFlagsB, SIMD colorMask, SIMD transparentColor) noexcept
{
    using MASK = SIMD::mask_type;
    constexpr MASK FLAG{BOOL_FLAG};
    constexpr SIMD SET_ALPHA{0xFF'00'00'00u};
    constexpr SIMD CLEAR_ALPHA{0x00'FF'FF'FFu};

    SIMD pixel = std::simd::unchecked_load(plane->AsU32Pointer(), SIMDNativePixelSize, std::simd::flag_aligned);
    pixel |= 0xFF'00'00'00; // Set to visible.
    const SIMD invisible = pixel & 0x00'FF'FF'FFu;

    const MASK colorKey = ((pixel & SIMD{COLOR_KEY_MASK}) | colorMask) == transparentColor;

    switch(TRANSPARENT)
    {
    case Renderer::TransparentIf::AlwaysNever: // Always/Never.
        pixel = std::simd::select(FLAG, invisible, pixel);
        break;

    case Renderer::TransparentIf::ColorKey: // Color Key.
        pixel = std::simd::select(colorKey == FLAG, invisible, pixel);
        break;

    case Renderer::TransparentIf::TransparencyBit: // Transparent Bit.
    {
        // TODO: currently decodeRGB555 make the pixel visible if the bit is set.
        // TODO: disable if not RGB555.
        const MASK mask = ((pixel & SET_ALPHA) != 0u) == FLAG;
        pixel = std::simd::select(mask, invisible, pixel);
        break;
    }

    case Renderer::TransparentIf::MatteFlag0: // Matte Flag 0.
    {
        // const SIMD matteVec = std::simd::unchecked_load<SIMD>(matteFlagsA, SIMDNativePixelSize, std::simd::flag_aligned);
        // Ugly cast but it appears bool is not value preserving when converting to integers.
        const SIMD matteVec = std::simd::unchecked_load<SIMD>(reinterpret_cast<const uint8_t*>(matteFlagsA), SIMDNativePixelSize, std::simd::flag_aligned);
        const MASK matte = matteVec != 0u;
        pixel = std::simd::select(matte == FLAG, invisible, pixel);
        break;
    }

    case Renderer::TransparentIf::MatteFlag1: // Matte Flag 1.
    {
        const SIMD matteVec = std::simd::unchecked_load<SIMD>(reinterpret_cast<const uint8_t*>(matteFlagsB), SIMDNativePixelSize, std::simd::flag_aligned);
        const MASK matte = matteVec != 0u;
        pixel = std::simd::select(matte == FLAG, invisible, pixel);
        break;
    }

    case Renderer::TransparentIf::MatteFlag0OrColorKey: // Matte Flag 0 or Color Key.
    {
        const SIMD matteVec = std::simd::unchecked_load<SIMD>(reinterpret_cast<const uint8_t*>(matteFlagsA), SIMDNativePixelSize, std::simd::flag_aligned);
        const MASK matte = matteVec != 0u;
        pixel = std::simd::select(matte == FLAG || colorKey == FLAG, invisible, pixel);
        break;
    }

    case Renderer::TransparentIf::MatteFlag1OrColorKey: // Matte Flag 1 or Color Key.
    {
        const SIMD matteVec = std::simd::unchecked_load<SIMD>(reinterpret_cast<const uint8_t*>(matteFlagsB), SIMDNativePixelSize, std::simd::flag_aligned);
        const MASK matte = matteVec != 0u;
        pixel = std::simd::select(matte == FLAG || colorKey == FLAG, invisible, pixel);
        break;
    }

    default: // Reserved.
        std::unreachable();
        break;
    }

    std::simd::unchecked_store(pixel, plane->AsU32Pointer(), SIMDNativePixelSize, std::simd::flag_aligned);
}
