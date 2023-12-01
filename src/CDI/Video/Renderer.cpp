#include "Renderer.hpp"

#include "../common/utils.hpp"

#include <cstring>
#include <iostream>

#define DCP_EXTRACT_COMMAND(inst) ((inst) & 0x00FF'FFFFu)
#define CLUT_COLOR_KEY(color) ((color) & 0x00FC'FCFCu) // V.5.7.2.2.

namespace Video
{

/** \brief Converts the 4-bits backdrop color to RGB.
 * \param rgb The RGB destination buffer.
 * \param color The 4 bit color code.
 */
static constexpr void backdropColorToARGB(uint8_t* rgb, const uint8_t color) noexcept
{
    // Background plane has no transparency (Green book V.5.13).
    const uint8_t c = (color & 0x08) ? 255 : 128;
    *rgb++ = (color & 0x04) ? c : 0; // Red.
    *rgb++ = (color & 0x02) ? c : 0; // Green.
    *rgb++ = (color & 0x01) ? c : 0; // Blue.
}

/** \brief Converts the 4-bits cursor color to ARGB.
 * \param argb The ARGB destination buffer.
 * \param color The 4 bit color code.
 */
static constexpr void cursorColorToARGB(uint8_t* argb, const uint8_t color) noexcept
{
    // Background plane has no transparency (Green book V.5.13).
    *argb++ = (color & 0x08) ? 0xFF : 128; // Alpha.
    *argb++ = (color & 0x04) ? 0xFF : 0; // Red.
    *argb++ = (color & 0x02) ? 0xFF : 0; // Green.
    *argb++ = (color & 0x01) ? 0xFF : 0; // Blue.
}

// /** \brief Color Look-Up Table for the backdrop and cursor colors. */
// static constexpr std::array<std::array<uint8_t, 3>, 16> backdropCursorCLUT{{
//     {{0x00, 0x00, 0x00}}, // Black.
//     {{0x00, 0x00, 0x80}}, // Half-brightness Blue.
//     {{0x00, 0x80, 0x00}}, // Half-brightness Green.
//     {{0x00, 0x80, 0x80}}, // Half-brightness Cyan.
//     {{0x80, 0x00, 0x00}}, // Half-brightness Red.
//     {{0x80, 0x00, 0x80}}, // Half-brightness Magenta.
//     {{0x80, 0x80, 0x00}}, // Half-brightness Yellow.
//     {{0x80, 0x80, 0x80}}, // Half-brightness White.
//     {{0x00, 0x00, 0x00}}, // Black.
//     {{0x00, 0x00, 0xFF}}, // Blue.
//     {{0x00, 0xFF, 0x00}}, // Green.
//     {{0x00, 0xFF, 0xFF}}, // Cyan.
//     {{0xFF, 0x00, 0x00}}, // Red.
//     {{0xFF, 0x00, 0xFF}}, // Magenta.
//     {{0xFF, 0xFF, 0x00}}, // Yellow.
//     {{0xFF, 0xFF, 0xFF}}, // White.
// }};

// static void backdropCursorColorToARGB2(uint8_t* rgb, const uint8_t color) noexcept
// {
//     memcpy(rgb, backdropCursorCLUT[color & 0x0Fu].data(), 3);
// }

/** \brief Sets the planes resolutions.
 *
 * This method must only be called after a frame has been drawn and before the next frame starts being drawn.
 * If this is called mid-frame, no error checks are performed to make sure the resolution matches.
 */
void Renderer::SetPlaneResolutions(uint16_t widthA, uint16_t widthB, uint16_t height) noexcept
{
    m_plane[A].m_width = m_screen.m_width = widthA;
    m_plane[B].m_width = widthB;
    m_plane[A].m_height = m_plane[B].m_height = m_backdropPlane.m_height = m_screen.m_height = height;
}

/** \brief Draws the next line to draw.
 * \param lineA Line A data.
 * \param lineB Line B data.
 * \return The number of bytes read from memory for each plane `<plane A, plane B>`.
 */
std::pair<uint16_t, uint16_t> Renderer::DrawLine(const uint8_t* lineA, const uint8_t* lineB) noexcept
{
    ResetMatte();

    const uint16_t bytesA = DecodeLinePlane<A>(nullptr, lineA); // nullptr because plane A can't decode RGB555.
    const uint16_t bytesB = DecodeLinePlane<B>(lineA, lineB);

    DecodeLineBackdrop();
    DecodeLineCursor();

    if(m_mix)
        OverlayMix<true>();
    else
        OverlayMix<false>();

    m_lineNumber++;
    return std::make_pair(bytesA, bytesB);
}

/** \brief To be called when the whole frame is drawn.
 * \return The final screen.
 *
 * This function renders the cursor and pastes it on the screen.
 * It also resets some members to prepare for the next frame.
 */
const Plane& Renderer::RenderFrame() noexcept
{
    m_lineNumber = 0;
    return m_screen;
}

/** \brief Decode the line of the given plane.
 * \param lineA Line A data if RGB555.
 * \param lineMain Line that will be decoded.
 * \return The number of bytes read from memory.
 *
 * lineA is only used when the decoding method is RGB555.
 */
template<Renderer::ImagePlane PLANE>
uint16_t Renderer::DecodeLinePlane(const uint8_t* lineA, const uint8_t* lineMain) noexcept
{
    if(m_codingMethod[PLANE] == ImageCodingMethod::OFF)
    {
        const size_t length = m_plane[PLANE].m_width * m_plane[PLANE].m_bpp;
        memset(m_plane[PLANE](m_lineNumber), 0, length);
        return 0;
    }

    const bool is4BPP = false; // TODO.

    const uint32_t* clut;
    if constexpr(PLANE == A)
        clut = m_codingMethod[A] == ICM(CLUT77) && m_clutSelect ? &m_clut[128] : m_clut.data();
    else
        clut = &m_clut[128];

    switch(m_imageType[PLANE])
    {
    case ImageType::Normal:
        return decodeBitmapLine(m_plane[PLANE](m_lineNumber), lineA, lineMain, m_plane[PLANE].m_width, clut, m_dyuvInitialValue[PLANE], m_codingMethod[PLANE]);

    case ImageType::RunLength:
        return decodeRunLengthLine(m_plane[PLANE](m_lineNumber), lineMain, m_plane[PLANE].m_width, clut, is4BPP);

    case ImageType::Mosaic:
        break;
    }

    // std::unreachable();
    return 0;
}

void Renderer::DecodeLineBackdrop() noexcept
{
    // The pixels of a line are all the same, so backdrop plane only contains the color of each line.
    backdropColorToARGB(m_backdropPlane(m_lineNumber), m_backdropColor);
}

void Renderer::DecodeLineCursor() noexcept
{
//     const uint16_t yPosition = controlRegisters[CursorPosition] >> 12 & 0x0003FF;
//     if(lineNumber < yPosition || lineNumber >= yPosition + 16)
//         return;
//
//     const uint8_t yAddress = lineNumber - yPosition;
//     uint8_t* pixels = cursorPlane(yAddress);
//
//     const uint8_t A = (controlRegisters[CursorControl] & 0x000008) ? 255 : 128;
//     const uint8_t R = (controlRegisters[CursorControl] & 0x000004) ? 255 : 0;
//     const uint8_t G = (controlRegisters[CursorControl] & 0x000002) ? 255 : 0;
//     const uint8_t B = (controlRegisters[CursorControl] & 0x000001) ? 255 : 0;
//
//     uint16_t mask = 1 << 15;
//     for(uint8_t i = 0, j = 0; i < 16; i++)
//     {
//         if(cursorPatterns[yAddress] & mask)
//         {
//             pixels[j++] = A;
//             pixels[j++] = R;
//             pixels[j++] = G;
//             pixels[j++] = B;
//         }
//         else
//         {
//             pixels[j] = 0;
//             j += 4;
//         }
//         mask >>= 1;
//     }
}

/** \brief Apply the given Image Contribution Factor to the given color component (V.5.9). */
static constexpr inline uint8_t computeICF(const int color, const int icf) noexcept
{
    return static_cast<uint8_t>(((icf * (color - 16)) / 63) + 16);
}

/** \brief Apply mixing to the given color components after ICF (V.5.9.1). */
static constexpr inline uint8_t mix(const int a, const int b) noexcept
{
    return limu8(a + b - 16);
}

/** \brief Overlays or mix all the planes to the final screen.
 * \tparam MIX true to use mixing, false to use overlay.
 */
template<bool MIX>
void Renderer::OverlayMix() noexcept
{
    uint8_t* screen = m_screen(m_lineNumber);
    uint8_t* planeA = m_plane[A](m_lineNumber);
    uint8_t* planeB = m_plane[B](m_lineNumber);

    uint8_t* background = m_backdropPlane(m_lineNumber);
    const uint8_t rbg = background[0];
    const uint8_t gbg = background[1];
    const uint8_t bbg = background[2];

    for(uint16_t i = 0; i < m_plane[A].m_width; i++) // TODO: width[B].
    {
        HandleMatte<A>(i);
        HandleMatte<B>(i);

        HandleTransparency<A>(planeA);
        HandleTransparency<B>(planeB);

        const uint8_t aa = *planeA++;
        const uint8_t ra = computeICF(*planeA++, m_icf[A]);
        const uint8_t ga = computeICF(*planeA++, m_icf[A]);
        const uint8_t ba = computeICF(*planeA++, m_icf[A]);

        const uint8_t ab = *planeB++;
        const uint8_t rb = computeICF(*planeB++, m_icf[B]);
        const uint8_t gb = computeICF(*planeB++, m_icf[B]);
        const uint8_t bb = computeICF(*planeB++, m_icf[B]);

        uint8_t afp, rfp, gfp, bfp, abp, rbp, gbp, bbp;
        if(m_planeOrder) // Plane B in front.
        {
            afp = ab;
            rfp = rb;
            gfp = gb;
            bfp = bb;

            abp = aa;
            rbp = ra;
            gbp = ga;
            bbp = ba;
        }
        else // Plane A in front.
        {
            afp = aa;
            rfp = ra;
            gfp = ga;
            bfp = ba;

            abp = ab;
            rbp = rb;
            gbp = gb;
            bbp = bb;
        }

        uint8_t r, g, b;
        if constexpr(MIX)
        {
            if(abp == 0) // When mixing transparent pixels are black (V.5.9.1).
            {
                rbp = 16;
                gbp = 16;
                bbp = 16;
            }

            if(afp == 0)
            {
                rfp = 16;
                gfp = 16;
                bfp = 16;
            }

            r = limu8(rbp + rfp - 16);
            g = limu8(gbp + gfp - 16);
            b = limu8(bbp + bfp - 16);
            // r = 16;
            // g = 16;
            // b = 16;
        }
        else
        {
            // Plane transparency is either 0 or 255.
            if(afp == 0 && abp == 0) // Front and back plane transparent: only show background.
            {
                r = rbg;
                g = gbg;
                b = bbg;
            }
            else if(afp == 0) // Front plane transparent: show back plane.
            {
                r = rbp;
                g = gbp;
                b = bbp;
            }
            else // Front plane visible: only show front plane.
            {
                r = rfp;
                g = gfp;
                b = bfp;
            }
        }

        *screen++ = r;
        *screen++ = g;
        *screen++ = b;

        // Apply ICF
        // When mixing transparent pixels are black (V.5.9.1).

        /*
        MCD212 figure 8-6
        For overlay/mix:
        - do the computation for a single pixel.
        - write it to the destination buffer with memcpy for mosaic and double resolution.
        */
    }
}

/** \brief Enables or disables the cursor plane.
 * \param enabled true to enable, false to disable.
 */
void Renderer::SetCursorEnabled(const bool enabled) noexcept
{
    m_cursorEnabled = enabled;
}

/** \brief Sets the position of the cursor plane.
 * \param x X position (double resolution).
 * \param y Y position (double resolution).
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
    m_cursorPatterns[line] = pattern;
}

/** \brief Handles the transparency of the current pixel for each plane.
 * \param pixel The ARGB pixel.
 */
template<Renderer::ImagePlane PLANE>
void Renderer::HandleTransparency(uint8_t pixel[4]) noexcept
{
    const bool boolean = (m_transparencyControl[PLANE] & 0x08u) == 0;
    const uint32_t color = (as<uint32_t>(pixel[1]) << 16) | (as<uint32_t>(pixel[2]) << 8) | as<uint32_t>(pixel[3]);
    const bool colorKey = CLUT_COLOR_KEY(color | m_maskColor[PLANE]) == CLUT_COLOR_KEY(m_transparentColor[PLANE] | m_maskColor[PLANE]); // TODO: don't compute if not CLUT.

    pixel[0] = 0xFF;

    switch(m_transparencyControl[PLANE] & 0x07u)
    {
    case 0b000: // Always/Never.
        pixel[0] = 0xFF + boolean; // Branchless.
        break;

    case 0b001: // Color Key.
        if(colorKey == boolean)
            pixel[0] = 0;
        break;

    case 0b010: // Transparent Bit.
        // TODO: currently decodeRGB555 make the pixel visible if the bit is set.
        // TODO: disable if not RGB555.
        if((pixel[0] == 0xFF) != boolean)
            pixel[0] = 0;
        break;

    case 0b011: // Matte Flag 0.
        if(m_matteFlags[0] == boolean)
            pixel[0] = 0;
        break;

    case 0b100: // Matte Flag 1.
        if(m_matteFlags[1] == boolean)
            pixel[0] = 0;
        break;

    case 0b101: // Matte Flag 0 or Color Key.
        if(m_matteFlags[0] == boolean || colorKey == boolean)
            pixel[0] = 0;
        break;

    case 0b110: // Matte Flag 1 or Color Key.
        if(m_matteFlags[1] == boolean || colorKey == boolean)
            pixel[0] = 0;
        break;

    case 0b111: // Reserved.
        break;
    }
}

/** \brief Apply Image Contribution Factor for the given plane. */
template<Renderer::ImagePlane PLANE>
void Renderer::ApplyICF(uint8_t& r, uint8_t& g, uint8_t& b) const noexcept
{
    r = computeICF(r, m_icf[PLANE]);
    g = computeICF(g, m_icf[PLANE]);
    b = computeICF(b, m_icf[PLANE]);
}

static constexpr inline uint8_t matteOp(const uint32_t matteCommand) noexcept
{
    return matteCommand >> 20 & 0xFu;
}

static constexpr inline bool matteMF(const uint32_t matteCommand) noexcept
{
    return (matteCommand & (1 << 16)) != 0;
}

static constexpr inline uint8_t matteICF(const uint32_t matteCommand) noexcept
{
    return matteCommand >> 10 & 0x3Fu;
}

static constexpr inline uint16_t matteXPosition(const uint32_t matteCommand) noexcept
{
    return matteCommand >> 1 & 0x3FFu; // TODO: handle double resolution.
}

/** \brief Called at the beginning of each line to reset the matte state.
 */
void Renderer::ResetMatte() noexcept
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
template<Renderer::ImagePlane PLANE>
void Renderer::HandleMatte(const uint16_t pos) noexcept
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
