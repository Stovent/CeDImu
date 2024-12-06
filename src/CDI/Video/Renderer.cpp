#include "Renderer.hpp"

#include "../common/panic.hpp"

namespace Video
{

/** \brief Sets the planes resolutions.
 *
 * This method must only be called after a frame has been drawn and before the next frame starts being drawn.
 * If this is called mid-frame, no error checks are performed to make sure the resolution matches.
 */
void Renderer::SetPlanesResolutions(uint16_t widthA, uint16_t widthB, uint16_t height) noexcept
{
    m_plane[A].m_width = m_screen.m_width = widthA;
    m_plane[B].m_width = widthB;
    m_plane[A].m_height = m_plane[B].m_height = m_backdropPlane.m_height = m_screen.m_height = height;
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

} // namespace Video
