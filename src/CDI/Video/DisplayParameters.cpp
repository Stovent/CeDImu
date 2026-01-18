#include "DisplayParameters.hpp"
#include "VideoDecoders.hpp"

#include "../common/panic.hpp"

namespace Video
{

/** \brief Configures the display format.
 * The input must only be a valid enum value.
 */
void DisplayParameters::SetDisplayFormat(const DisplayFormat display, const bool highResolution, bool fps60) noexcept
{
    if(!isValidDisplayFormat(display))
        panic("Invalid display format {}", static_cast<int>(display));

    m_displayFormat = display;
    m_highResolution = highResolution;
    m_60FPS = fps60;
}

bool DisplayParameters::isValidDisplayFormat(const DisplayFormat display) noexcept
{
    return display == DisplayFormat::NTSCMonitor || display == DisplayFormat::NTSCTV || display == DisplayFormat::PAL;
}

/** \brief Enables or disables the cursor plane.
 * \param enabled true to enable, false to disable.
 */
void DisplayParameters::SetCursorEnabled(const bool enabled) noexcept
{
    m_cursorEnabled = enabled;
}

/** \brief Set the resolution of the cursor.
 * \param doubleResolution true for double/high resolution, false for normal resolution.
 */
void DisplayParameters::SetCursorResolution(const bool doubleResolution) noexcept
{
    if(doubleResolution)
        panic("Unsupported cursor double resolution");
    m_cursorDoubleResolution = doubleResolution;
}

/** \brief Sets the position of the cursor plane.
 * \param x X position (double resolution).
 * \param y Y position (normal resolution).
 */
void DisplayParameters::SetCursorPosition(const uint16_t x, const uint16_t y) noexcept
{
    m_cursorX = x;
    m_cursorY = y;
}

/** \brief Sets the cursor plane color.
 * \param argb 4-bits color code (see backdrop colors).
 */
void DisplayParameters::SetCursorColor(const uint8_t argb) noexcept
{
    m_cursorColor = argb & 0x0Fu;
}

/** \brief Sets the pattern of the given line of the cursor plane.
 * \param line The line to set the pattern of (from 0 to 15).
 * \param pattern The pattern to set.
 */
void DisplayParameters::SetCursorPattern(const uint8_t line, const uint16_t pattern) noexcept
{
    m_cursorPatterns.at(line) = pattern;
}

/** \brief Configures cursor blinking.
 * \param type false is on/off, true is on/complement.
 * \param periodOn ON period (zero not allowed).
 * \param periodOff OFF period (if zero, blink is disabled).
 */
void DisplayParameters::SetCursorBlink(bool type, uint8_t periodOn, uint8_t periodOff) noexcept
{
    if(periodOn == 0 && periodOff != 0) // If ON is 0 AND blink is enabled (OFF not 0).
        panic("Cursor blink ON period cannot be 0");

    m_cursorBlinkType = type;
    m_cursorBlinkOn = periodOn;
    m_cursorBlinkOff = periodOff;
}

/** \brief Checks if the given ICM for both planes are valid according to Green Book V.4.4.8. */
bool DisplayParameters::isAllowedImageCodingCombination(ImageCodingMethod planeA, ImageCodingMethod planeB) noexcept
{
    using enum ImageCodingMethod;

    if(planeB == RGB555 && planeA != OFF)
        return false;

    if((planeA == CLUT8 || planeA == CLUT77) && (planeB != DYUV && planeB != OFF))
        return false;

    // If I implement QHY, if(planeB == QHY && (planeA != DYUV && planeA != OFF)) return false;
    return true;
}

} // namespace Video
