#ifndef CDI_VIDEO_RENDERER_HPP
#define CDI_VIDEO_RENDERER_HPP

#include "../common/utils.hpp"
#include "Video/VideoCommon.hpp"

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
class Renderer
{
public:
    enum ImagePlane : uint8_t
    {
        A = 0,
        B = 1,
    };

    /** \brief The possible values for the alpha byte of pixels. */
    enum PixelIntensity : uint8_t
    {
        PIXEL_TRANSPARENT = 0x00, /**< Pixel is transparent. */
        PIXEL_HALF_INTENSITY = 0x7F, /**< Pixel is half-intensity. */
        PIXEL_FULL_INTENSITY = 0xFF, /**< Pixel is full-intensity. */
    };

    enum class DisplayFormat
    {
        NTSCMonitor, /**< 525 lines (360 x 240). */
        NTSCTV, /**< 525 lines (384 x 240). */
        PAL, /**< 625 lines (384 x 280). */
    };

    // This enum exists to explicitely indicate that double resolution is 4 bits per pixels, and 8 for high res.
    enum class BitsPerPixel
    {
        Normal8, /**< 8 bits/pixel, Normal Resolution. */
        Double4, /**< 4 bits/pixel, Double or High Resolution. */
        High8, /**< 8 bits/pixel, High Resolution. */
    };

    enum class ImageType
    {
        Normal,
        RunLength,
        Mosaic,
    };

    static constexpr Pixel BLACK_PIXEL{0x00'10'10'10};

    Renderer() {}
    virtual ~Renderer() noexcept {}

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    constexpr DisplayFormat GetDisplayFormat() const noexcept { return m_displayFormat; }
    void SetDisplayFormat(DisplayFormat display, bool highResolution, bool fps60) noexcept;
    static bool isValidDisplayFormat(DisplayFormat display) noexcept;

    static constexpr uint16_t getDisplayWidth(DisplayFormat display) noexcept
    {
        return display == DisplayFormat::NTSCMonitor ? 360 : 384;
    }

    static constexpr uint16_t getDisplayHeight(DisplayFormat display) noexcept
    {
        return display == DisplayFormat::PAL ? 280 : 240;
    }

    constexpr uint16_t GetDisplayHeight() const noexcept
    {
        const uint16_t height = getDisplayHeight(m_displayFormat);
        return m_highResolution ? height << 1 : height;
    }

    constexpr bool Is360Pixels() const noexcept
    {
        return m_screen.m_width == 720;
    }

    void IncrementCursorTime(double ns) noexcept;
    virtual std::pair<uint16_t, uint16_t> DrawLine(const uint8_t* lineA, const uint8_t* lineB, uint16_t lineNumber) noexcept = 0;
    const Plane& RenderFrame() noexcept;

    template<ImagePlane PLANE>
    bool ExecuteDCPInstruction(uint32_t instruction) noexcept;

    void SetCursorEnabled(bool enabled) noexcept;
    void SetCursorResolution(bool doubleResolution) noexcept;
    void SetCursorPosition(uint16_t x, uint16_t y) noexcept;
    void SetCursorColor(uint8_t argb) noexcept;
    void SetCursorPattern(uint8_t line, uint16_t pattern) noexcept;
    void SetCursorBlink(bool type, uint8_t periodOn, uint8_t periodOff) noexcept;

    Plane m_screen{};
    std::array<Plane, 2> m_plane{Plane{}, Plane{}};
    Plane m_backdropPlane{1, Plane::MAX_HEIGHT, Plane::MAX_HEIGHT};
    Plane m_cursorPlane{Plane::CURSOR_WIDTH, Plane::CURSOR_HEIGHT, Plane::CURSOR_SIZE}; /**< The alpha is 0, 127 or 255. */

    std::array<uint32_t, 2> m_dyuvInitialValue{};
    bool m_planeOrder{}; /**< true for B in front of A, false for A in front of B. */
    uint8_t m_clutBank : 2{};
    std::array<uint32_t, 256> m_clut{}; /**< RGB data in the lowest 24 bits. */

    // Image Coding Methods.
    bool m_clutSelectHigh{};
    bool m_matteNumber{}; /**< false for 1 matte, true for 2. */
    bool m_externalVideo{};
    std::array<ImageCodingMethod, 2> m_codingMethod{ImageCodingMethod::OFF, ImageCodingMethod::OFF};
    static bool isAllowedImageCodingCombination(ImageCodingMethod planeA, ImageCodingMethod planeB) noexcept;

    // Display Parameters.
    std::array<ImageType, 2> m_imageType{ImageType::Normal, ImageType::Normal};
    std::array<uint8_t, 2> m_pixelRepeatFactor{1, 1}; /**< (Raw pixel decoding V.26). */
    std::array<BitsPerPixel, 2> m_bps{BitsPerPixel::Normal8, BitsPerPixel::Normal8}; /**< Bits per pixel. */
    std::array<bool, 2> m_holdEnabled{}; /**< Pixel Hold Enabled (overlay V.25). */
    std::array<uint8_t, 2> m_holdFactor{1, 1}; /**< Pixel Hold Factor (overlay V.25). */

    // Backdrop.
    uint8_t m_backdropColor : 4{}; /**< YRGB color code. */
    static constexpr Pixel backdropCursorColorToPixel(uint8_t color) noexcept;

    // Cursor.
    bool m_cursorEnabled{};
    bool m_cursorDoubleResolution{};
    uint16_t m_cursorX{}; /**< Double resolution. */
    uint16_t m_cursorY{}; /**< Normal resolution. */
    uint8_t m_cursorColor : 4{}; /**< YRGB color code. */
    std::array<uint16_t, 16> m_cursorPatterns{};
    bool m_cursorBlinkType{}; /**< false is on/off, true is on/complement. */
    uint8_t m_cursorBlinkOn : 3{1}; /**< ON period (zero not allowed). */
    uint8_t m_cursorBlinkOff : 3{}; /**< OFF period (if zero, blink is disabled). */

    // Image Contribution Factor.
    std::array<uint8_t, 2> m_icf{};
    std::array<std::array<uint8_t, Plane::MAX_WIDTH>, 2> m_icfLine{}; /**< ICF for the whole line. */

    // Transparency.
    bool m_mix{true}; /**< true when mixing is enabled. */
    std::array<uint8_t, 2> m_transparencyControl{};
    std::array<uint32_t, 2> m_transparentColorRgb{}; /**< RGB data in the lowest 24 bits. */
    std::array<uint32_t, 2> m_maskColorRgb{}; /**< RGB data in the lowest 24 bits. */

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
    static constexpr size_t MATTE_NUM = 8; // Should never have to change.
    static constexpr size_t MATTE_HALF = MATTE_NUM / 2;
    std::array<uint32_t, MATTE_NUM> m_matteControl{};
    std::array<bool, 2> m_matteFlags{};
    std::array<uint8_t, 2> m_nextMatte{};
    constexpr void ResetMatte() noexcept;
    template<ImagePlane PLANE> constexpr void HandleMatte(uint16_t pos) noexcept;

    void HandleMatteAndTransparency(uint16_t lineNumber) noexcept;
    template<TransparentIf TRANSPARENCY_A, bool FLAG_A>
    void HandleMatteAndTransparencyDispatchB(uint16_t lineNumber) noexcept;
    template<TransparentIf TRANSPARENCY_A, bool FLAG_A, TransparentIf TRANSPARENCY_B, bool FLAG_B>
    void HandleMatteAndTransparencyLoop(uint16_t lineNumber) noexcept;

    enum ControlProgramInstruction : uint8_t
    {
        NoOperation = 0x10,
        LoadControlTableLineStartPointer = 0x20,
        LoadDisplayLineStartPointer = 0x40,
        SignalScanLine = 0x60,
        LoadDisplayParameters = 0x78,
        CLUTBank0  = 0x80,
        CLUTBank63 = 0xBF,
        SelectImageCodingMethod = 0xC0,
        LoadTransparencyControl = 0xC1,
        LoadPlaneOrder = 0xC2,
        SetCLUTBank = 0xC3,
        LoadTransparentColorA = 0xC4,
        LoadTransparentColorB = 0xC6,
        LoadMaskColorA = 0xC7,
        LoadMaskColorB = 0xC9,
        LoadDYUVStartValueA = 0xCA,
        LoadDYUVStartValueB = 0xCB,
        LoadMatteRegister0 = 0xD0,
        LoadMatteRegister7 = 0xD7,
        LoadBackdropColor = 0xD8,
        LoadMosaicFactorA = 0xD9,
        LoadMosaicFactorB = 0xDA,
        LoadImageContributionFactorA = 0xDB,
        LoadImageContributionFactorB = 0xDC,
    };

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
    DisplayFormat m_displayFormat{DisplayFormat::PAL}; /**< Used to select 360/384 width and 240/280 height. */
    bool m_highResolution{false}; /**< True for 480/560, false for 240/280 pixels height. */
    bool m_60FPS{false}; /**< False for 50FPS, true for 60FPS. */

    virtual void DrawCursor() noexcept = 0;
    void DrawLineBackdrop() noexcept
    {
        // The pixels of a line are all the same, so backdrop plane only contains the color of each line.
        *m_backdropPlane.GetLinePointer(m_lineNumber) = backdropCursorColorToPixel(m_backdropColor);
    }

    /** \brief Returns the lowest 24-bits that contains the DCP command. */
    static constexpr uint32_t dcpExtractCommand(const uint32_t inst) { return inst & 0x00FF'FFFFu; }

    /** \brief Masks the given color to the actually used bytes (V.5.7.2.2). */
    static constexpr uint32_t clutColorKey(const uint32_t color) { return color & 0x00FC'FCFCu; }
};

/** \brief Converts the 4-bits backdrop/cursor color to a Pixel.
 * \param color The 4 bit color code.
 * \returns The Pixel.
 */
constexpr Pixel Renderer::backdropCursorColorToPixel(const uint8_t color) noexcept
{
    // Background plane has no transparency (Green book V.5.13).
    Pixel argb = 0xFF'00'00'00; // Set transparency for cursor plane.
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
