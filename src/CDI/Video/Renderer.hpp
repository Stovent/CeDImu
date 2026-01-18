#ifndef CDI_VIDEO_RENDERER_HPP
#define CDI_VIDEO_RENDERER_HPP

#include "../common/utils.hpp"
#include "DisplayParameters.hpp"
#include "VideoCommon.hpp"

#include <array>
#include <cstdint>
#include <utility>

namespace Video
{

static constexpr bool matteMF(const uint32_t matteCommand) noexcept
{
    return bit<16>(matteCommand);
}

/** \brief CD-i video renderer base class as described in the Green Book.
 *
 * Every array member with 2 elements means its meant to be index based on the plane number \ref ImagePlane.
 *
 * TODO:
 * - V.25/V.26 Pixel repeat on pixel decoding, pixel hold on overlay.
 * - Should there be a reset method?
 *
 * TODO optimizations:
 * - RGB55 is optimisable as it's only a single plane, check how to make it in both software and SIMD.
 */
class Renderer : public DisplayParameters
{
public:
    /** \brief The possible values for the alpha byte of pixels. */
    enum PixelIntensity : uint8_t
    {
        PIXEL_TRANSPARENT = 0x00, /**< Pixel is transparent. */
        PIXEL_HALF_INTENSITY = 0x7F, /**< Pixel is half-intensity. */
        PIXEL_FULL_INTENSITY = 0xFF, /**< Pixel is full-intensity. */
    };

    static constexpr Pixel BLACK_PIXEL{0x00'10'10'10};

    Renderer() {}
    virtual ~Renderer() noexcept {}

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    constexpr bool Is360Pixels() const noexcept
    {
        return m_screen.m_width == 720;
    }

    void IncrementCursorTime(double ns) noexcept;
    virtual std::pair<uint16_t, uint16_t> DrawLine(const uint8_t* lineA, const uint8_t* lineB, uint16_t lineNumber) noexcept = 0;
    const Plane& RenderFrame() noexcept;

    Plane m_screen{};
    std::array<Plane, 2> m_plane{Plane{}, Plane{}};
    Plane m_backdropPlane{1, Plane::MAX_HEIGHT, Plane::MAX_HEIGHT};
    Plane m_cursorPlane{Plane::CURSOR_WIDTH, Plane::CURSOR_HEIGHT, Plane::CURSOR_SIZE}; /**< The alpha is 0, 127 or 255. */

    static constexpr Pixel backdropCursorColorToPixel(uint8_t color) noexcept;

    // Image Contribution Factor.
    std::array<std::array<uint8_t, Plane::MAX_WIDTH>, 2> m_icfLine{}; /**< ICF for the whole line. */

    // Transparency.
    /** \brief The condition for transparency, excluding the boolean bit.
     * Pixel is transparent if the mechanism is equal to the inverted bit 3.
     */
    enum class TransparentIf
    {
        AlwaysNever = 0b000,
        ColorKey    = 0b001,
        TransparencyBit = 0b010,
        MatteFlag0  = 0b011,
        MatteFlag1  = 0b100,
        MatteFlag0OrColorKey = 0b101,
        MatteFlag1OrColorKey = 0b110,
    };

    template<ImagePlane PLANE, TransparentIf TRANSPARENT, bool BOOL_FLAG>
    constexpr void HandleTransparency(Pixel& pixel) noexcept;

    // Matte (Region of the MCD212).
    std::array<bool, 2> m_matteFlags{};
    std::array<uint8_t, 2> m_nextMatte{};
    constexpr void ResetMatte() noexcept;
    template<ImagePlane PLANE> void HandleMatte(uint16_t pos) noexcept;

    void HandleMatteAndTransparency(uint16_t lineNumber) noexcept;
    template<TransparentIf TRANSPARENCY_A, bool FLAG_A>
    void HandleMatteAndTransparencyDispatchB(uint16_t lineNumber) noexcept;
    template<TransparentIf TRANSPARENCY_A, bool FLAG_A, TransparentIf TRANSPARENCY_B, bool FLAG_B>
    void HandleMatteAndTransparencyLoop(uint16_t lineNumber) noexcept;

    static constexpr double DELTA_50FPS = 240'000'000.; /**< GB VII.2.3.4.2 GC_Blnk. */
    static constexpr double DELTA_60FPS = 200'000'000.; /**< GB VII.2.3.4.2 GC_Blnk. */

protected:
    double m_cursorTime{0.0}; /**< Keeps track of the emulated time for cursor blink. */
    bool m_cursorIsOn{true}; /**< Keeps the state of the cursor (ON or OFF/complement). true when ON. */
    constexpr Pixel GetCursorColor() const noexcept
    {
        Pixel color = backdropCursorColorToPixel(m_cursorColor);
        if(m_cursorBlinkOff != 0 && !m_cursorIsOn)
        {
            if(m_cursorBlinkType) // Complement.
                color = color.Complement();
            else
                color = BLACK_PIXEL;
        }
        return color;
    }

    uint16_t m_lineNumber{}; /**< Current line being drawn, starts at 0. Handled by the caller. */

    virtual void DrawCursor() noexcept = 0;
    void DrawLineBackdrop() noexcept
    {
        // The pixels of a line are all the same, so backdrop plane only contains the color of each line.
        *m_backdropPlane.GetLinePointer(m_lineNumber) = backdropCursorColorToPixel(m_backdropColor);
    }

    /** \brief Masks the given color to the actually used bytes (V.5.7.2.2). */
    static constexpr uint32_t clutColorKey(const uint32_t color) { return color & 0x00FC'FCFCu; }

    static constexpr uint8_t matteOp(const uint32_t matteCommand) noexcept { return bits<20, 23>(matteCommand); }
    static constexpr uint8_t matteICF(const uint32_t matteCommand) noexcept { return bits<10, 15>(matteCommand); }
    static constexpr uint16_t matteXPosition(const uint32_t matteCommand) noexcept { return bits<0, 9>(matteCommand); }
};

/** \brief Converts the 4-bits backdrop/cursor color to a Pixel.
 * \param color The 4 bit color code.
 * \returns The Pixel.
 */
constexpr Pixel Renderer::backdropCursorColorToPixel(const uint8_t color) noexcept
{
    // Background plane has no transparency (Green book V.5.13).
    Pixel argb = 0xFF'00'00'00; // Background and cursor color pixels are always visible.
    const uint8_t c = bit<3>(color) ? Renderer::PIXEL_FULL_INTENSITY : Renderer::PIXEL_HALF_INTENSITY;
    if(bit<2>(color)) argb.r = c; // Red.
    if(bit<1>(color)) argb.g = c; // Green.
    if(bit<0>(color)) argb.b = c; // Blue.
    return argb;
}

/** \brief Called at the beginning of each line to reset the matte state.
 *
 * TODO: how does ICF behave after the frame?
 * - is it reset to m_icf[A/B] on each line?
 * - does it keep the latest value for all the next line? (so m_icfLine[A/B].fill(m_icfLine[A/B][last]);)
 */
constexpr void Renderer::ResetMatte() noexcept
{
    m_matteFlags.fill(false);
    // m_icfLine[A].fill(0);
    // m_icfLine[B].fill(0);

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
}

} // namespace Video

#endif // CDI_VIDEO_RENDERER_HPP
