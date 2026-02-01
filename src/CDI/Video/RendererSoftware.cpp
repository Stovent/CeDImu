/** \file RendererSoftware.cpp
 * \brief RendererSoftware implementation file.
 */

#include "RendererSoftware.hpp"
#include "VideoDecoders.hpp"

#include "../common/panic.hpp"
#include "../common/utils.hpp"

#include <cstring>

namespace Video
{

/** \brief Draws the next line to draw.
 * \param lineA Line A data.
 * \param lineB Line B data.
 * \param lineNumber The line number to draw (starting at 0).
 * \return The number of bytes read from memory for each plane `<plane A, plane B>`.
 */
std::pair<uint16_t, uint16_t> RendererSoftware::DrawLineImpl(const uint8_t* lineA, const uint8_t* lineB) noexcept
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
uint16_t RendererSoftware::DrawLinePlane(const uint8_t* lineMain, const uint8_t* lineA) noexcept
{
    if(m_codingMethod[PLANE] == ImageCodingMethod::OFF)
    {
        std::fill_n(m_plane[PLANE].GetLinePointer(m_lineNumber), m_plane[PLANE].m_width, Pixel{0});
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
                return decodeBitmapLine<720>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineA, lineMain, clut, m_dyuvInitialValue[PLANE], icm);
            else
                return decodeBitmapLine<768>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineA, lineMain, clut, m_dyuvInitialValue[PLANE], icm);
        else
            if(Is360Pixels())
                return decodeBitmapLine<360>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineA, lineMain, clut, m_dyuvInitialValue[PLANE], icm);
            else
                return decodeBitmapLine<384>(m_plane[PLANE].GetLinePointer(m_lineNumber), lineA, lineMain, clut, m_dyuvInitialValue[PLANE], icm);

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

void RendererSoftware::DrawCursor() noexcept
{
    // Technically speaking the cursor is drawn when the drawing line number is the cursor's one (because video
    // is outputted continuously line by line). But for here maybe we don't care.
    const Pixel color = GetCursorColor();

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
    dst.a = Renderer::PIXEL_FULL_INTENSITY;
    dst.r = limu8(a.r + b.r - 16);
    dst.g = limu8(a.g + b.g - 16);
    dst.b = limu8(a.b + b.b - 16);
}

/** \brief Overlays or mix all the planes to the final screen.
 * \tparam MIX true to use mixing, false to use overlay.
 * \tparam PLANE_ORDER true when plane B in front of plane A, false for A in front of B.
 */
template<bool MIX, bool PLANE_ORDER>
void RendererSoftware::OverlayMix() noexcept
{
    Pixel* screen = m_screen.GetLinePointer(m_lineNumber);
    Pixel* planeA = m_plane[A].GetLinePointer(m_lineNumber);
    Pixel* planeB = m_plane[B].GetLinePointer(m_lineNumber);
    const Pixel backdrop = *m_backdropPlane.GetLinePointer(m_lineNumber);

    HandleMatteAndTransparency(m_lineNumber);

    for(uint16_t i = 0; i < m_plane[A].m_width; i++) // Both planes always have the same width.
    {
        Pixel a = *planeA++;
        Pixel b = *planeB++;

        applyICF(a, m_icfLine[A][i]);
        applyICF(b, m_icfLine[B][i]);

        Pixel fp, bp;
        if constexpr(PLANE_ORDER) // Plane B in front.
        {
            fp = b;
            bp = a;
        }
        else // Plane A in front.
        {
            fp = a;
            bp = b;
        }

        // Plane transparency is either 0 or 255.
        // Backdrop is always visible.
        Pixel pixel;
        if constexpr(MIX)
        {
            // When mixing transparent pixels are black (V.5.9.1).
            if(fp.a == 0 && bp.a == 0) // Front and back planes transparent: only show background.
            {
                pixel = backdrop;
            }
            else if(fp.a == 0) // Front plane transparent: only back plane.
            {
                pixel = bp;
            }
            else if(bp.a == 0) // Back plane transparent: only front plane.
            {
                pixel = fp;
            }
            else // Both planes visible: mix them.
            {
                mix(pixel, fp, bp);
            }
        }
        else // Overlay.
        {
            if(fp.a == 0 && bp.a == 0) // Front and back planes transparent: only show background.
            {
                pixel = backdrop;
            }
            else if(fp.a == 0) // Front plane transparent: show back plane.
            {
                pixel = bp;
            }
            else // Front plane visible: only show front plane.
            {
                pixel = fp;
            }
        }
        *screen++ = pixel; // Should always have the transparency component to 255.
    }
}

/** \brief Applies matte and transparency to the two video planes. */
void RendererSoftware::HandleMatteAndTransparency(const uint16_t lineNumber) noexcept
{
    const bool booleanA = !bit<3>(m_transparencyControl[A]);
    const uint8_t controlA = bits<0, 2>(m_transparencyControl[A]);

    switch(static_cast<TransparentIf>(controlA))
    {
    case TransparentIf::AlwaysNever: // Always/Never.
        if(booleanA)
            HandleMatteAndTransparencyDispatchB<TransparentIf::AlwaysNever, true>(lineNumber);
        else
            HandleMatteAndTransparencyDispatchB<TransparentIf::AlwaysNever, false>(lineNumber);
        break;

    case TransparentIf::ColorKey: // Color Key.
        if(booleanA)
            HandleMatteAndTransparencyDispatchB<TransparentIf::ColorKey, true>(lineNumber);
        else
            HandleMatteAndTransparencyDispatchB<TransparentIf::ColorKey, false>(lineNumber);
        break;

    case TransparentIf::TransparencyBit: // Transparent Bit.
        // TODO: currently decodeRGB555 make the pixel visible if the bit is set.
        // TODO: disable if not RGB555.
        if(booleanA)
            HandleMatteAndTransparencyDispatchB<TransparentIf::TransparencyBit, true>(lineNumber);
        else
            HandleMatteAndTransparencyDispatchB<TransparentIf::TransparencyBit, false>(lineNumber);
        break;

    case TransparentIf::MatteFlag0: // Matte Flag 0.
        if(booleanA)
            HandleMatteAndTransparencyDispatchB<TransparentIf::MatteFlag0, true>(lineNumber);
        else
            HandleMatteAndTransparencyDispatchB<TransparentIf::MatteFlag0, false>(lineNumber);
        break;

    case TransparentIf::MatteFlag1: // Matte Flag 1.
        if(booleanA)
            HandleMatteAndTransparencyDispatchB<TransparentIf::MatteFlag1, true>(lineNumber);
        else
            HandleMatteAndTransparencyDispatchB<TransparentIf::MatteFlag1, false>(lineNumber);
        break;

    case TransparentIf::MatteFlag0OrColorKey: // Matte Flag 0 or Color Key.
        if(booleanA)
            HandleMatteAndTransparencyDispatchB<TransparentIf::MatteFlag0OrColorKey, true>(lineNumber);
        else
            HandleMatteAndTransparencyDispatchB<TransparentIf::MatteFlag0OrColorKey, false>(lineNumber);
        break;

    case TransparentIf::MatteFlag1OrColorKey: // Matte Flag 1 or Color Key.
        if(booleanA)
            HandleMatteAndTransparencyDispatchB<TransparentIf::MatteFlag1OrColorKey, true>(lineNumber);
        else
            HandleMatteAndTransparencyDispatchB<TransparentIf::MatteFlag1OrColorKey, false>(lineNumber);
        break;

    default: // Reserved.
        std::unreachable();
        break;
    }
}

/** \brief Takes the template transparency control of A and mixes it with B's and handle them both. */
template<Renderer::TransparentIf TRANSPARENCY_A, bool FLAG_A>
void RendererSoftware::HandleMatteAndTransparencyDispatchB(const uint16_t lineNumber) noexcept
{
    const bool booleanB = !bit<3>(m_transparencyControl[B]);
    const uint8_t controlB = bits<0, 2>(m_transparencyControl[B]);

    switch(static_cast<TransparentIf>(controlB))
    {
    case TransparentIf::AlwaysNever: // Always/Never.
        if(booleanB)
            HandleMatteAndTransparencyLoop<TRANSPARENCY_A, FLAG_A, TransparentIf::AlwaysNever, true>(lineNumber);
        else
            HandleMatteAndTransparencyLoop<TRANSPARENCY_A, FLAG_A, TransparentIf::AlwaysNever, false>(lineNumber);
        break;

    case TransparentIf::ColorKey: // Color Key.
        if(booleanB)
            HandleMatteAndTransparencyLoop<TRANSPARENCY_A, FLAG_A, TransparentIf::ColorKey, true>(lineNumber);
        else
            HandleMatteAndTransparencyLoop<TRANSPARENCY_A, FLAG_A, TransparentIf::ColorKey, false>(lineNumber);
        break;

    case TransparentIf::TransparencyBit: // Transparent Bit.
        // TODO: currently decodeRGB555 make the pixel visible if the bit is set.
        // TODO: disable if not RGB555.
        if(booleanB)
            HandleMatteAndTransparencyLoop<TRANSPARENCY_A, FLAG_A, TransparentIf::TransparencyBit, true>(lineNumber);
        else
            HandleMatteAndTransparencyLoop<TRANSPARENCY_A, FLAG_A, TransparentIf::TransparencyBit, false>(lineNumber);
        break;

    case TransparentIf::MatteFlag0: // Matte Flag 0.
        if(booleanB)
            HandleMatteAndTransparencyLoop<TRANSPARENCY_A, FLAG_A, TransparentIf::MatteFlag0, true>(lineNumber);
        else
            HandleMatteAndTransparencyLoop<TRANSPARENCY_A, FLAG_A, TransparentIf::MatteFlag0, false>(lineNumber);
        break;

    case TransparentIf::MatteFlag1: // Matte Flag 1.
        if(booleanB)
            HandleMatteAndTransparencyLoop<TRANSPARENCY_A, FLAG_A, TransparentIf::MatteFlag1, true>(lineNumber);
        else
            HandleMatteAndTransparencyLoop<TRANSPARENCY_A, FLAG_A, TransparentIf::MatteFlag1, false>(lineNumber);
        break;

    case TransparentIf::MatteFlag0OrColorKey: // Matte Flag 0 or Color Key.
        if(booleanB)
            HandleMatteAndTransparencyLoop<TRANSPARENCY_A, FLAG_A, TransparentIf::MatteFlag0OrColorKey, true>(lineNumber);
        else
            HandleMatteAndTransparencyLoop<TRANSPARENCY_A, FLAG_A, TransparentIf::MatteFlag0OrColorKey, false>(lineNumber);
        break;

    case TransparentIf::MatteFlag1OrColorKey: // Matte Flag 1 or Color Key.
        if(booleanB)
            HandleMatteAndTransparencyLoop<TRANSPARENCY_A, FLAG_A, TransparentIf::MatteFlag1OrColorKey, true>(lineNumber);
        else
            HandleMatteAndTransparencyLoop<TRANSPARENCY_A, FLAG_A, TransparentIf::MatteFlag1OrColorKey, false>(lineNumber);
        break;

    default: // Reserved.
        std::unreachable();
        break;
    }
}

/** \brief Actually handles matte and transparency for both planes statically. */
template<Renderer::TransparentIf TRANSPARENCY_A, bool FLAG_A, Renderer::TransparentIf TRANSPARENCY_B, bool FLAG_B>
void RendererSoftware::HandleMatteAndTransparencyLoop(const uint16_t lineNumber) noexcept
{
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

    Pixel* planeA = m_plane[A].GetLinePointer(lineNumber);
    Pixel* planeB = m_plane[B].GetLinePointer(lineNumber);

    for(uint16_t i = 0; i < m_plane[A].m_width; ++i) // Plane B has the same width.
    {
        HandleMatte<A>(i);
        HandleMatte<B>(i);
        m_icfLine[A][i] = m_icf[A];
        m_icfLine[B][i] = m_icf[B];
        HandleTransparency<A, TRANSPARENCY_A, FLAG_A>(*planeA++);
        HandleTransparency<B, TRANSPARENCY_B, FLAG_B>(*planeB++);
    }
}

/** \brief Handles the transparency of the current pixel for each plane.
 * \tparam PLANE The video plane to operate on.
 * \tparam TRANSPARENT The low 3-bits of the transparency instruction.
 * \tparam BOOL_FLAG True if bit 3 is 0, false if bit 3 is 1.
 * \param pixel The ARGB pixel.
 * TODO: do not compute colorKey if not CLUT.
 */
template<ImagePlane PLANE, Renderer::TransparentIf TRANSPARENT, bool BOOL_FLAG>
constexpr void RendererSoftware::HandleTransparency(Pixel& pixel) noexcept
{
    uint32_t color = static_cast<uint32_t>(pixel) & 0x00'FF'FF'FF;
    color = clutColorKey(color | m_maskColorRgb[PLANE]);
    const bool colorKey = color == clutColorKey(m_transparentColorRgb[PLANE] | m_maskColorRgb[PLANE]);

    pixel.a = PIXEL_FULL_INTENSITY;

    switch(TRANSPARENT)
    {
    case TransparentIf::AlwaysNever: // Always/Never.
        pixel.a = static_cast<uint8_t>(PIXEL_FULL_INTENSITY + BOOL_FLAG); // Branchless.
        break;

    case TransparentIf::ColorKey: // Color Key.
        if(colorKey == BOOL_FLAG)
            pixel.a = PIXEL_TRANSPARENT;
        break;

    case TransparentIf::TransparencyBit: // Transparent Bit.
        // TODO: currently decodeRGB555 make the pixel visible if the bit is set.
        // TODO: disable if not RGB555.
        if((pixel.a == PIXEL_FULL_INTENSITY) != BOOL_FLAG)
            pixel.a = PIXEL_TRANSPARENT;
        break;

    case TransparentIf::MatteFlag0: // Matte Flag 0.
        if(m_matteFlags[A] == BOOL_FLAG)
            pixel.a = PIXEL_TRANSPARENT;
        break;

    case TransparentIf::MatteFlag1: // Matte Flag 1.
        if(m_matteFlags[B] == BOOL_FLAG)
            pixel.a = PIXEL_TRANSPARENT;
        break;

    case TransparentIf::MatteFlag0OrColorKey: // Matte Flag 0 or Color Key.
        if(m_matteFlags[A] == BOOL_FLAG || colorKey == BOOL_FLAG)
            pixel.a = PIXEL_TRANSPARENT;
        break;

    case TransparentIf::MatteFlag1OrColorKey: // Matte Flag 1 or Color Key.
        if(m_matteFlags[B] == BOOL_FLAG || colorKey == BOOL_FLAG)
            pixel.a = PIXEL_TRANSPARENT;
        break;

    default: // Reserved.
        std::unreachable();
        break;
    }
}

/** \brief Handles the matte flags for the given plane at the given pixel position.
 * \param pos The current pixel position (in double resolution).
 */
template<ImagePlane PLANE>
void RendererSoftware::HandleMatte(uint16_t pos) noexcept
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
    if(matteXPosition(command) != pos)
        return;

    ++m_nextMatte[PLANE];

    /* TODO: matte flag index changed should be based on its index. V.5.10.2 note 8 */
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
