#include "Renderer.hpp"
#include "VideoDecoders.hpp"

#include "../common/panic.hpp"

namespace Video
{

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

std::pair<uint16_t, uint16_t> Renderer::DrawLine(const uint8_t* lineA, const uint8_t* lineB, uint16_t lineNumber) noexcept
{
    m_lineNumber = lineNumber;
    if(m_lineNumber == 0) [[unlikely]]
    {
        const uint16_t width = getDisplayWidth(m_displayFormat);
        const uint16_t height = GetDisplayHeight();

        m_screen.m_width = m_plane[A].m_width = m_plane[B].m_width = width * 2;
        m_screen.m_height = m_plane[A].m_height = m_plane[B].m_height = m_backdropPlane.m_height = height;
    }

    ResetMatte();

    DrawLineBackdrop();

    return DrawLineImpl(lineA, lineB);
}

/** \brief To be called when the whole frame is drawn.
 * \return The final screen.
 *
 * This function renders the cursor and pastes it on the screen.
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

} // namespace Video
