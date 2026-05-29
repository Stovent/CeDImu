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
std::pair<uint16_t, uint16_t> RendererSIMD::DrawLineImpl(const uint8_t* lineA, const uint8_t* lineB) noexcept
{
    uint16_t bytesA = DrawLinePlane<A>(lineA, nullptr); // nullptr because plane A can't decode RGB555.
    const uint16_t bytesB = DrawLinePlane<B>(lineB, lineA);
    if(m_codingMethod[B] == ImageCodingMethod::RGB555)
        bytesA = bytesB;

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
                {
                    nextChange0 = matteXPosition(m_matteControl[nextMatte0]);
                    if(nextChange0 <= x) // Sometimes the next register has a lower position.
                        nextChange0 = m_screen.m_width;
                }
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

#if __has_include(<simd>)
#include "RendererSIMDstd.cpp"
#else
#include "RendererSIMDexp.cpp"
#endif

} // namespace Video
