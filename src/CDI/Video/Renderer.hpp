#ifndef CDI_VIDEO_RENDERER_HPP
#define CDI_VIDEO_RENDERER_HPP

#include "../common/utils.hpp"
#include "Video/VideoCommon.hpp"

#include <array>
#include <cstdint>
#include <utility>

namespace Video
{

static constexpr uint32_t argbArrayToU32(uint8_t* pixels) noexcept
{
    return (as<uint32_t>(pixels[1]) << 16) | (as<uint32_t>(pixels[2]) << 8) | as<uint32_t>(pixels[3]);
}

static constexpr bool matteMF(const uint32_t matteCommand) noexcept
{
    return bit<16>(matteCommand);
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
    return bits<1, 9>(matteCommand); // TODO: handle double resolution.
}

/** \brief CD-i video renderer base class as described in the Green Book.
 *
 * Every array member with 2 elements means its meant to be index based on the plane number \ref ImagePlane.
 *
 * I absolutely really need to implement multi resolutions because my code just assumes normal res for all the planes.
 *
 * TODO:
 * - Handle Width, Height, double resolution.
 * - Should the renderer manage the line count and draw itself the final frame and the cursor when reached,
 *   or let it to the user of the renderer with a `Plane RenderFrame();` method ?
 * - Let all members public because the MCD212/VSD has access to them all ?
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

    enum class Resolution
    {
        Normal, /**< Same resolution as display format. */
        Double, /**< Double horizontal resolution, normal vertical resolution. */
        High, /**< Double horizontal and vertical resolution. */
    };

    static constexpr Pixel BLACK_PIXEL{0x00'10'10'10};

    Renderer() {}
    virtual ~Renderer() noexcept {}

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    virtual std::pair<uint16_t, uint16_t> DrawLine(const uint8_t* lineA, const uint8_t* lineB) noexcept = 0;
    virtual const Plane& RenderFrame() noexcept = 0;

    template<ImagePlane PLANE>
    bool ExecuteDCPInstruction(uint32_t instruction) noexcept;

    void SetDisplayResolution(DisplayFormat display, Resolution resolution) noexcept;
    static std::pair<size_t, size_t> getPixelResolution(DisplayFormat display, Resolution resolution) noexcept;

    void SetPlanesResolutions(uint16_t widthA, uint16_t widthB, uint16_t height) noexcept;
    static bool isValidWidth(uint16_t width) noexcept;
    static bool isValidHeight(uint16_t height) noexcept;
    static bool validateResolution(uint16_t widthA, uint16_t widthB, uint16_t height) noexcept;

    void SetCursorEnabled(bool enabled) noexcept;
    void SetCursorResolution(bool doubleResolution) noexcept;
    void SetCursorPosition(uint16_t x, uint16_t y) noexcept;
    void SetCursorColor(uint8_t argb) noexcept;
    void SetCursorPattern(uint8_t line, uint16_t pattern) noexcept;

    enum class ImageType
    {
        Normal,
        RunLength,
        Mosaic,
    };

    // TODO: organize and order the members correctly.

    Plane m_screen{384, 280};
    // Plane m_screen{384, 280, SIMDAlign(Plane::RGB_MAX_SIZE)}; // align for SIMD test.
    std::array<Plane, 2> m_plane{Plane{384, 280}, Plane{384, 280}};
    Plane m_backdropPlane{1, Plane::MAX_HEIGHT, Plane::MAX_HEIGHT};
    Plane m_cursorPlane{Plane::CURSOR_WIDTH, Plane::CURSOR_HEIGHT, Plane::CURSOR_SIZE}; /**< The alpha is 0, 127 or 255. */

    uint16_t m_lineNumber{}; /**< Current line being drawn, starts at 0. */

    std::array<uint32_t, 2> m_dyuvInitialValue{};
    bool m_planeOrder{}; /**< true for B in front of A, false for A in front of B. */
    uint8_t m_clutBank : 2{};
    std::array<uint32_t, 256> m_clut{}; /**< RGB data in the lowest 24 bits. */

    // Image Coding Methods.
    bool m_clutSelectHigh{};
    bool m_matteNumber{}; /**< false for 1 matte, true for 2. */
    bool m_externalVideo{};
    std::array<ImageCodingMethod, 2> m_codingMethod{ImageCodingMethod::OFF, ImageCodingMethod::OFF};

    // Display Parameters.
    std::array<ImageType, 2> m_imageType{ImageType::Normal, ImageType::Normal};
    std::array<uint8_t, 2> m_pixelRepeatFactor{1, 1}; /**< (Raw pixel decoding V.26). */
    std::array<bool, 2> m_bps{}; /**< false for 8 bits per pixels, true for 4 bps. */
    std::array<bool, 2> m_holdEnabled{}; /**< Pixel Hold Enabled (overlay V.25). */
    std::array<uint8_t, 2> m_holdFactor{1, 1}; /**< Pixel Hold Factor (overlay V.25). */

    // Backdrop.
    uint8_t m_backdropColor : 4{}; /**< YRGB color code. */
    static Pixel backdropCursorColorToPixel(uint8_t color) noexcept;

    // Cursor.
    bool m_cursorEnabled{};
    bool m_cursorDoubleResolution{};
    uint16_t m_cursorX{}; /**< Double resolution. */
    uint16_t m_cursorY{}; /**< Normal resolution. */
    uint8_t m_cursorColor : 4{}; /**< YRGB color code. */
    std::array<uint16_t, 16> m_cursorPatterns{};
    // TODO: implement blink.

    // Image Contribution Factor.
    std::array<uint8_t, 2> m_icf{};
    // TODO: ensure SIMD alignment
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

protected:
    /** \brief Returns the lowest 24-bits that contains the DCP command. */
    static constexpr uint32_t dcpExtractCommand(const uint32_t inst) { return inst & 0x00FF'FFFFu; }

    /** \brief Masks the given color to the actually used bytes (V.5.7.2.2). */
    static constexpr uint32_t clutColorKey(const uint32_t color) { return color & 0x00FC'FCFCu; }
};

/** \brief Called at the beginning of each line to reset the matte state.
 */
constexpr void Renderer::ResetMatte() noexcept
{
    m_matteFlags.fill(false);

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

/** \brief Handles the matte flags for the given plane at the given pixel position.
 * \param pos The current pixel position (in normal resolution).
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

#endif // CDI_VIDEO_RENDERER_HPP
