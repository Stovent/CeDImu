#include "Renderer.hpp"

#include "../common/panic.hpp"

namespace Video
{

/** \brief Configures the display format and resolution.
 * The input must only be a valid enum value.
 */
void Renderer::SetDisplayResolution(DisplayFormat display, Resolution resolution) noexcept
{
    if((display != DisplayFormat::NTSCMonitor && display != DisplayFormat::NTSCTV && display != DisplayFormat::PAL) &&
       (resolution != Resolution::Normal && resolution != Resolution::Double && resolution != Resolution::High)
    )
        panic("Invalid display resolution");

    const std::pair<size_t, size_t> sizes = getPixelResolution(display, resolution);
    SetPlanesResolutions(sizes.first, sizes.first, sizes.second);
}

/** \brief Returns the `<width, height>` in pixels of the given display resolution combination.
 * The input must only be a valid enum value.
 */
std::pair<size_t, size_t> Renderer::getPixelResolution(DisplayFormat display, Resolution resolution) noexcept
{
    size_t width = 0;
    size_t height = 0;

    switch(display)
    {
    case DisplayFormat::NTSCMonitor:
        width = 360;
        height = 240;
        break;

    case DisplayFormat::NTSCTV:
        width = 384;
        height = 240;
        break;

    case DisplayFormat::PAL:
        width = 384;
        height = 280;
        break;
    }

    switch(resolution)
    {
    case Resolution::Normal:
        break;

    case Resolution::Double:
        width *= 2;
        break;

    case Resolution::High:
        width *= 2;
        height *= 2;
        break;
    }

    return std::make_pair(width, height);
}

/** \brief Sets the planes resolutions.
 *
 * This method must only be called after a frame has been drawn and before the next frame starts being drawn.
 * If this is called mid-frame, no error checks are performed to make sure the resolution matches.
 */
void Renderer::SetPlanesResolutions(uint16_t widthA, uint16_t widthB, uint16_t height) noexcept
{
    if(!validateResolution(widthA, widthB, height))
        panic("Invalid resolution combination: {} {} {}", widthA, widthB, height);

    m_plane[A].m_width = m_screen.m_width = widthA;
    m_plane[B].m_width = widthB;
    m_plane[A].m_height = m_plane[B].m_height = m_backdropPlane.m_height = m_screen.m_height = height;
}

bool Renderer::isValidWidth(uint16_t width) noexcept
{
    return (width == 360 || width == 384 || width == 720 || width == 768);
}

bool Renderer::isValidHeight(uint16_t height) noexcept
{
    return (height == 240 || height == 280 || height == 480 || height == 560);
}

bool Renderer::validateResolution(uint16_t widthA, uint16_t widthB, uint16_t height) noexcept
{
    return isValidWidth(widthA) && isValidWidth(widthB) && isValidHeight(height);
}

/** \brief Enables or disables the cursor plane.
 * \param enabled true to enable, false to disable.
 */
void Renderer::SetCursorEnabled(const bool enabled) noexcept
{
    m_cursorEnabled = enabled;
}

/** \brief Set the resolution of the cursor.
 * \param doubleResolution true for double/high resolution, false for normal resolution.
 */
void Renderer::SetCursorResolution(const bool doubleResolution) noexcept
{
    if(doubleResolution)
        panic("Unsupported cursor double resolution");
    m_cursorDoubleResolution = doubleResolution;
}

/** \brief Sets the position of the cursor plane.
 * \param x X position (double resolution).
 * \param y Y position (normal resolution).
 */
void Renderer::SetCursorPosition(const uint16_t x, const uint16_t y) noexcept
{
    m_cursorX = x;
    m_cursorY = y;
}

/** \brief Sets the cursor plane color.
 * \param color 4-bits color code (see backdrop colors).
 */
void Renderer::SetCursorColor(const uint8_t color) noexcept
{
    m_cursorColor = color & 0x0Fu;
}

/** \brief Sets the pattern of the given line of the cursor plane.
 * \param line The line to set the pattern of (from 0 to 15).
 * \param pattern The pattern to set.
 */
void Renderer::SetCursorPattern(const uint8_t line, const uint16_t pattern) noexcept
{
    m_cursorPatterns.at(line) = pattern;
}

/** \brief Converts the 4-bits backdrop color to a Pixel.
 * \param color The 4 bit color code.
 * \returns The Pixel.
 */
Pixel Renderer::backdropCursorColorToPixel(const uint8_t color) noexcept
{
    // Background plane has no transparency (Green book V.5.13).
    Pixel argb = 0xFF'00'00'00; // Set transparency for cursor plane.
    const uint8_t c = bit<3>(color) ? Renderer::PIXEL_FULL_INTENSITY : Renderer::PIXEL_HALF_INTENSITY;
    if(bit<2>(color)) argb.r = c; // Red.
    if(bit<1>(color)) argb.g = c; // Green.
    if(bit<0>(color)) argb.b = c; // Blue.
    return argb;
}

/** \brief Handles the transparency of the current pixel for each plane.
 * \tparam PLANE The video plane to operate on.
 * \tparam TRANSPARENT The low 3-bits of the transparency instruction.
 * \tparam BOOL_FLAG True if bit 3 is 0, false if bit 3 is 1.
 * \param pixel The ARGB pixel.
 * TODO: do not compute colorKey if not CLUT.
 */
template<Renderer::ImagePlane PLANE, Renderer::TransparentIf TRANSPARENT, bool BOOL_FLAG>
constexpr void Renderer::HandleTransparency(Pixel& pixel) noexcept
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

/** \brief Applies matte and transparency to the two video planes. */
void Renderer::HandleMatteAndTransparency(const uint16_t lineNumber) noexcept
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
void Renderer::HandleMatteAndTransparencyDispatchB(const uint16_t lineNumber) noexcept
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
void Renderer::HandleMatteAndTransparencyLoop(const uint16_t lineNumber) noexcept
{
    Pixel* planeA = m_plane[A].GetLinePointer(lineNumber);
    Pixel* planeB = m_plane[B].GetLinePointer(lineNumber);

    for(uint16_t i = 0; i < m_plane[A].m_width; ++i) // TODO: width[B].
    {
        HandleMatte<A>(i);
        HandleMatte<B>(i);
        m_icfLine[A][i] = m_icf[A];
        m_icfLine[B][i] = m_icf[B];
        HandleTransparency<A, TRANSPARENCY_A, FLAG_A>(*planeA++);
        HandleTransparency<B, TRANSPARENCY_B, FLAG_B>(*planeB++);
    }
}

} // namespace Video
