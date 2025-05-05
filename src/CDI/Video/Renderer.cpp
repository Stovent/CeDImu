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

} // namespace Video
