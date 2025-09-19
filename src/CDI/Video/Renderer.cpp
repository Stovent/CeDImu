#include "Renderer.hpp"
#include "VideoDecoders.hpp"

#include "../common/panic.hpp"

namespace Video
{

/** \brief Configures the display format.
 * The input must only be a valid enum value.
 */
void Renderer::SetDisplayFormat(const DisplayFormat display, const bool highResolution, bool fps60) noexcept
{
    if(!isValidDisplayFormat(display))
        panic("Invalid display format {}", static_cast<int>(display));

    m_displayFormat = display;
    m_highResolution = highResolution;
    m_60FPS = fps60;
}

bool Renderer::isValidDisplayFormat(const DisplayFormat display) noexcept
{
    return display == DisplayFormat::NTSCMonitor || display == DisplayFormat::NTSCTV || display == DisplayFormat::PAL;
}

/** \brief Increments the internal time of the cursor blink.
 * \param ns The time in nanosecond.
 */
void Renderer::IncrementCursorTime(const double ns) noexcept
{
    if(!m_cursorEnabled || m_cursorBlinkOff == 0)
        return; // OFF == 0 means ON indefinitely.

    double delta = m_60FPS ? DELTA_60FPS : DELTA_50FPS;
    if(m_cursorIsOn)
        delta *= m_cursorBlinkOn;
    else
        delta *= m_cursorBlinkOff;

    m_cursorTime += ns;
    if(m_cursorTime >= delta)
    {
        m_cursorTime -= delta;
        m_cursorIsOn = !m_cursorIsOn;
    }
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
        paste(m_screen.data(), m_screen.m_width, m_screen.m_height,
              m_cursorPlane.data(), m_cursorPlane.m_width, m_cursorPlane.m_height,
              m_cursorX, m_cursorY);
    }

    return m_screen;
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

/** \brief Configures cursor blinking.
 * \param type false is on/off, true is on/complement.
 * \param periodOn ON period (zero not allowed).
 * \param periodOff OFF period (if zero, blink is disabled).
 */
void Renderer::SetCursorBlink(bool type, uint8_t periodOn, uint8_t periodOff) noexcept
{
    if(periodOn == 0 && periodOff != 0) // If ON is 0 AND blink is enabled (OFF not 0).
        panic("Cursor blink ON period cannot be 0");

    m_cursorBlinkType = type;
    m_cursorBlinkOn = periodOn;
    m_cursorBlinkOff = periodOff;
}

/** \brief Checks if the given ICM for both planes are valid according to Green Book V.4.4.8. */
bool Renderer::isAllowedImageCodingCombination(ImageCodingMethod planeA, ImageCodingMethod planeB) noexcept
{
    using enum ImageCodingMethod;

    if(planeB == RGB555 && planeA != OFF)
        return false;

    if((planeA == CLUT8 || planeA == CLUT77) && planeB != DYUV)
        return false;

    // If I implement QHY, if(planeB == QHY && planeA != DYUV) return false;
    return true;
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
    return bits<0, 9>(matteCommand);
}

/** \brief Handles the matte flags for the given plane at the given pixel position.
 * \param pos The current pixel position (in double resolution).
 */
template<Renderer::ImagePlane PLANE> constexpr void Renderer::HandleMatte(uint16_t pos) noexcept
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
