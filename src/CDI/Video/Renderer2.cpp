#include "Renderer.hpp"

// #include "../common/utils.hpp"

#include <cstring>
// #include <iostream>

namespace Video
{

/** \brief Draws the next line to draw.
 * \param lineA Line A data.
 * \param lineB Line B data.
 * \return The number of bytes read from memory for each plane <plane A, plane B>.
 */
std::pair<uint16_t, uint16_t> Renderer::DrawLine2(const uint8_t* lineA, const uint8_t* lineB) noexcept
{
    ResetMatte();

    const std::pair<uint16_t, uint16_t> bytes = DecodeLines2(lineA, lineB);

    m_lineNumber++;
    return bytes;
}

/*
Idea for a pixel-based path (decode+mix+matte per pixel):
iterators for the input image stores ?

switch(ImageCodingMethod)
{
case:
    decodeBackground();
    if(cursor)
        decodeCursor();
    for(width)
    {
        decodePixelA();
        decodePixelB();
        HandleMatte();
        HandleTransparency();
        OverlayMix();
    }
}

for pixel repeat/runlength:
    do
    {
        memcpy(dstPlane, uint32_t pixel, 4);
    } while(--pixelFactor>0) // adjust to real algo.
*/

/** \brief Decode the line of the given plane.
 * \param lineA Line A data if RGB555.
 * \param lineMain Line that will be decoded.
 * \return The number of bytes read from memory.
 *
 * lineA is only used when the decoding method is RGB555.
 */
std::pair<uint16_t, uint16_t> Renderer::DecodeLines2(const uint8_t* , const uint8_t* ) noexcept
{
    uint16_t bytesA = 0;
    uint16_t bytesB = 0;

    if(m_codingMethod[A] == ImageCodingMethod::OFF)
    {
        const size_t length = m_plane[A].m_width * m_plane[A].m_bpp;
        memset(m_plane[A](m_lineNumber), 0, length);
    }

    if(m_codingMethod[B] == ImageCodingMethod::OFF)
    {
        const size_t length = m_plane[B].m_width * m_plane[B].m_bpp;
        memset(m_plane[B](m_lineNumber), 0, length);
    }

//     const bool is4BPP = false; // TODO.
//
//     const uint32_t* clut;
//     if constexpr(PLANE == A)
//         clut = m_codingMethod[A] == ICM(CLUT77) && m_clutSelect ? &m_clut[128] : m_clut.data();
//     else
//         clut = &m_clut[128];
//
//     switch(m_imageType[PLANE])
//     {
//     case ImageType::Normal:
//         return decodeBitmapLine(m_plane[PLANE](m_lineNumber), lineA, lineMain, m_width[PLANE], clut, m_dyuvInitialValue[PLANE], m_codingMethod[PLANE]);
//
//     case ImageType::RunLength:
//         return decodeRunLengthLine(m_plane[PLANE](m_lineNumber), lineMain, m_width[PLANE], clut, is4BPP);
//
//     case ImageType::Mosaic:
//         break;
//     }

    // std::unreachable();
    return std::make_pair(bytesA, bytesB);
}

} // namespace Video
