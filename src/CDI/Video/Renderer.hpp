#ifndef CDI_CORES_RENDERER_HPP
#define CDI_CORES_RENDERER_HPP

#include "../common/Video.hpp"

#include <array>
#include <cstdint>
#include <span>
#include <utility>

namespace Video
{

/** \brief CD-i video renderer as described in the Green Book.
 *
 * Every array member with 2 elements means its meant to be index based on the plane number \ref ImagePlane.
 *
 * TODO:
 * - Handle Width, Height, double resolution.
 * - Should the renderer manage the line count and draw itself the final frame and the cursor when reached,
 *   or let it to the user of the renderer with a `Plane RenderFrame();` method ?
 * - Let all members public because the MCD212/VSD has access to them all ?
 * - V.25/V.26 Pixel repeat on pixel decoding, pixel hold on overlay.
 */
class Renderer
{
public:
    enum ImagePlane : bool
    {
        A = false,
        B = true,
    };

    Renderer() {}

    // void Reset() noexcept;
    void SetPlanesResolutions(uint16_t widthA, uint16_t widthB, uint16_t height) noexcept;

    std::pair<uint16_t, uint16_t> DrawLine(const uint8_t* lineA, const uint8_t* lineB) noexcept;
    std::pair<uint16_t, uint16_t> DrawLine2(const uint8_t* lineA, const uint8_t* lineB) noexcept;
    const Plane& RenderFrame() noexcept;

    template<ImagePlane PLANE>
    bool ExecuteDCPInstruction(uint32_t instruction) noexcept;

    void SetCursorEnabled(bool enabled) noexcept;
    void SetCursorPosition(uint16_t x, uint16_t y) noexcept;
    void SetCursorColor(uint8_t argb) noexcept;
    void SetCursorPattern(uint8_t line, uint16_t pattern) noexcept;

    Plane m_screen{3, 384, 280, Plane::RGB_MAX_SIZE};
    std::array<Plane, 2> m_plane{Plane{4, 384, 280}, Plane{4, 384, 280}};
    Plane m_backdropPlane{3, 1, Plane::MAX_HEIGHT, Plane::MAX_HEIGHT * 3};
    Plane m_cursorPlane{4, Plane::CURSOR_WIDTH, Plane::CURSOR_HEIGHT, Plane::CURSOR_ARGB_SIZE}; // TODO: also make the cursor RGB like the background ?

    enum class ImageType
    {
        Normal,
        RunLength,
        Mosaic,
    };

    template<ImagePlane PLANE>
    uint16_t DecodeLinePlane(const uint8_t* lineA, const uint8_t* lineMain) noexcept;
    std::pair<uint16_t, uint16_t> DecodeLines2(const uint8_t* lineA, const uint8_t* lineB) noexcept;
    void DecodeLineBackdrop() noexcept;
    void DecodeLineCursor() noexcept;
    template<bool MIX> void OverlayMix() noexcept;

    // template<ImagePlane PLANE>
    // uint8_t DecodePixel(uint8_t* dst, const uint8_t* lineA, const uint8_t* lineB, uint32_t& previousDYUV) noexcept;
    // void DecodeNormalImage() noexcept;
    // void DecodeNormalImage(, uint8_t mosaicFactor) noexcept;
    // void DecodeRunLengthImage() noexcept;
    // void DecodeMosaicImage() noexcept;

    // TODO: organize and order the members correctly.
    // TODO: Split into dedicated directory and separate files (Display Control) ?

    uint16_t m_lineNumber{}; /**< Current line being drawn, starts at 0. */

    std::array<uint32_t, 2> m_dyuvInitialValue{};
    bool m_planeOrder{}; /**< true for B in front of A, false for A in front of B. */
    uint8_t m_clutBank{};
    std::array<uint32_t, 256> m_clut{}; /**< RGB data in the lowest 24 bits. */

    // Image Coding Methods.
    bool m_clutSelect{};
    bool m_matteNumber{}; /**< false for 1 matte, true for 2. */
    // bool m_externalVideo{};
    std::array<ImageCodingMethod, 2> m_codingMethod{ImageCodingMethod::OFF, ImageCodingMethod::OFF};

    // Display Parameters.
    std::array<ImageType, 2> m_imageType{ImageType::Normal, ImageType::Normal};
    std::array<uint8_t, 2> m_pixelRepeatFactor{1, 1}; /**< (Raw pixel decoding V.26). */
    std::array<bool, 2> m_bps{}; /**< false for 8 bits per pixels, true for 4 bps. */
    std::array<bool, 2> m_holdEnabled{}; /**< Pixel Hold Enabled (overlay V.25). */
    std::array<uint8_t, 2> m_holdFactor{}; /**< Pixel Hold Factor (overlay V.25). */

    // Transparency.
    bool m_mix{true}; /**< true when mixing is enabled. */
    std::array<uint8_t, 2> m_transparencyControl{};
    std::array<uint32_t, 2> m_transparentColorRgb{}; /**< RGB data in the lowest 24 bits. */
    std::array<uint32_t, 2> m_maskColorRgb{}; /**< RGB data in the lowest 24 bits. */
    template<ImagePlane PLANE> void HandleTransparency(uint8_t pixel[4]) noexcept;

    // Backdrop.
    uint8_t m_backdropColor{};

    // Cursor.
    bool m_cursorEnabled{};
    uint16_t m_cursorX{};
    uint16_t m_cursorY{};
    uint8_t m_cursorColor{};
    std::array<uint16_t, 16> m_cursorPatterns{};

    // Image Contribution Factor.
    std::array<uint8_t, 2> m_icf{};
    template<ImagePlane PLANE> void ApplyICF(uint8_t& r, uint8_t& g, uint8_t& b) const noexcept;

    // Matte (Region of the MCD212).
    static constexpr size_t MATTE_NUM = 8; // Should never have to change.
    static constexpr size_t MATTE_HALF = MATTE_NUM / 2;
    std::array<uint32_t, MATTE_NUM> m_matteControl{};
    std::array<bool, 2> m_matteFlags{};
    std::array<uint8_t, 2> m_nextMatte{};
    void ResetMatte() noexcept;
    template<ImagePlane PLANE> void HandleMatte(uint16_t pos) noexcept;

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

private:
    /** \brief Returns the lowest 24-bits that contains the DCP command. */
    static uint32_t dcpExtractCommand(const uint32_t inst) { return inst & 0x00FF'FFFFu; }
    /** \brief Masks the given color to the actually used bytes (V.5.7.2.2). */
    static uint32_t clutColorKey(const uint32_t color) { return color & 0x00FC'FCFCu; }
};

} // namespace Video

#endif // CDI_CORES_RENDERER_HPP
