#ifndef CDI_VIDEO_DISPLAYPARAMETERS_HPP
#define CDI_VIDEO_DISPLAYPARAMETERS_HPP

#include "VideoCommon.hpp"
#include "../common/utils.hpp"

#include <array>
#include <cstdint>
#include <utility>

namespace Video
{

/** \brief The display paramaters as modifiable by the Display Control Program.
 *
 * Every array member with 2 elements means its meant to be index based on the plane number \ref ImagePlane.
 */
class DisplayParameters
{
public:
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

    template<ImagePlane PLANE>
    bool ExecuteDCPInstruction(uint32_t instruction) noexcept;

    void SetCursorEnabled(bool enabled) noexcept;
    void SetCursorResolution(bool doubleResolution) noexcept;
    void SetCursorPosition(uint16_t x, uint16_t y) noexcept;
    void SetCursorColor(uint8_t argb) noexcept;
    void SetCursorPattern(uint8_t line, uint16_t pattern) noexcept;
    void SetCursorBlink(bool type, uint8_t periodOn, uint8_t periodOff) noexcept;

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

    // Transparency.
    bool m_mix{true}; /**< true when mixing is enabled. */
    std::array<uint8_t, 2> m_transparencyControl{};
    std::array<uint32_t, 2> m_transparentColorRgb{}; /**< RGB data in the lowest 24 bits. */
    std::array<uint32_t, 2> m_maskColorRgb{}; /**< RGB data in the lowest 24 bits. */

    // Matte (Region of the MCD212).
    static constexpr size_t MATTE_NUM = 8; // Should never have to change.
    static constexpr size_t MATTE_HALF = MATTE_NUM / 2;
    std::array<uint32_t, MATTE_NUM> m_matteControl{};

protected:
    DisplayFormat m_displayFormat{DisplayFormat::PAL}; /**< Used to select 360/384 width and 240/280 height. */
    bool m_highResolution{false}; /**< True for 480/560, false for 240/280 pixels height. */
    bool m_60FPS{false}; /**< False for 50FPS, true for 60FPS. */
};

} // namespace Video

#endif // CDI_VIDEO_DISPLAYPARAMETERS_HPP
