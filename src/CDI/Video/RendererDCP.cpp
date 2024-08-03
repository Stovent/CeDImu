#include "Renderer.hpp"
#include "../common/utils.hpp"

#include <iostream>

namespace Video
{

/** \brief Figure V.48 */
static constexpr ImageCodingMethod decodeCodingMethod0(const uint8_t method) noexcept
{
    switch(method)
    {
    case 0x01:
        return ImageCodingMethod::CLUT8;

    case 0x03:
        return ImageCodingMethod::CLUT7;

    case 0x04:
        return ImageCodingMethod::CLUT77;

    case 0x05:
        return ImageCodingMethod::DYUV;

    case 0x0B:
        return ImageCodingMethod::CLUT4;

    default:
        return ImageCodingMethod::OFF;
    }
}

/** \brief Figure V.48 */
static constexpr ImageCodingMethod decodeCodingMethod1(const uint8_t method) noexcept
{
    switch (method)
    {
    case 0x01:
        return ImageCodingMethod::RGB555;

    case 0x03:
        return ImageCodingMethod::CLUT7;

    case 0x05:
        return ImageCodingMethod::DYUV;

    case 0x0B:
        return ImageCodingMethod::CLUT4;

    default:
        return ImageCodingMethod::OFF;
    }
}

/** \brief Figure V.49. */
static constexpr Renderer::ImageType decodeImageType(const uint8_t type) noexcept
{
    if(type <= 1)
        return Renderer::ImageType::Normal;
    if(type == 2)
        return Renderer::ImageType::RunLength;
    std::cerr << "Unsupported Mosaic image type\n";
    return Renderer::ImageType::Mosaic;
}

/** \brief Executes the given DCP intruction.
 * \tparam LCT true for LCT, false for FCT.
 * \tparam PLANE The plane executing the instruction.
 * \return true if a video interrupt has to be generated, false otherwise.
 *
 * If an unknown instruction is provided, nothing is done.
 * The following instructions are not handled by this function:
 * - 0x20 Load control table line start pointer
 * - 0x40 Load display line start pointer
 */
template<Renderer::ImagePlane PLANE>
bool Renderer::ExecuteDCPInstruction(const uint32_t instruction) noexcept
{
    const uint8_t code = instruction >> 24;

    if(code >= CLUTBank0 && code <= CLUTBank63) // CLUT bank.
    {
        // In theory CLUT loading should be checked based on the coding methods (5.5 CLUT update).
        if(PLANE == A || m_clutBank >= 2)
        {
            uint8_t clutAddr = m_clutBank << 6;
            clutAddr += code - CLUTBank0;
            m_clut[clutAddr] = dcpExtractCommand(instruction);
        }
        return false;
    }

    if(code >= LoadMatteRegister0 && code <= LoadMatteRegister7) // Matte.
    {
        m_matteControl[code - LoadMatteRegister0] = dcpExtractCommand(instruction);
        return false;
    }

    if constexpr(PLANE == A) // Plane A-only instructions.
    {
        switch(code)
        {
        case SelectImageCodingMethod: // Select image coding methods.
            m_clutSelect = bit<22>(instruction);
            m_matteNumber = bit<19>(instruction);
            m_codingMethod[B] = decodeCodingMethod1(bits<8, 11>(instruction));
            m_codingMethod[A] = decodeCodingMethod0(bits<0, 3>(instruction));
            // TODO: what to do with external video enabled?
            m_externalVideo = bit<18>(instruction);
            return false;

        case LoadTransparencyControl: // Load transparency control information.
            m_mix = !bit<23>(instruction);
            m_transparencyControl[B] = bits<8, 11>(instruction);
            m_transparencyControl[A] = bits<0, 3>(instruction);
            return false;

        case LoadPlaneOrder: // Load plane order.
            m_planeOrder = bit<0>(instruction);
            return false;

        case LoadTransparentColorA: // Load transparent color for plane A.
            m_transparentColorRgb[A] = dcpExtractCommand(instruction);
            return false;

        case LoadMaskColorA: // Load mask color for plane A.
            m_maskColorRgb[A] = dcpExtractCommand(instruction);
            return false;

        case LoadDYUVStartValueA: // Load DYUV start value for plane A.
            m_dyuvInitialValue[A] = dcpExtractCommand(instruction);
            return false;

        case LoadBackdropColor: // Load backdrop color.
            m_backdropColor = bits<0, 3>(instruction);
            return false;

        case LoadMosaicFactorA: // Load mosaic pixel hold factor for A.
            m_holdEnabled[A] = bit<23>(instruction);
            m_holdFactor[A] = instruction;
            return false;

        case LoadImageContributionFactorA: // Load image contribution factor for A.
            m_icf[A] = bits<0, 5>(instruction);
            return false;
        }
    }
    else // Plane B-only instructions.
    {
        switch(code)
        {
        case LoadTransparentColorB: // Load transparent color for plane B.
            m_transparentColorRgb[B] = dcpExtractCommand(instruction);
            return false;

        case LoadMaskColorB: // Load mask color for plane B.
            m_maskColorRgb[B] = dcpExtractCommand(instruction);
            return false;

        case LoadDYUVStartValueB: // Load DYUV start value for plane B.
            m_dyuvInitialValue[B] = dcpExtractCommand(instruction);
            return false;

        case LoadMosaicFactorB: // Load mosaic pixel hold factor for B.
            m_holdEnabled[B] = bit<23>(instruction);
            m_holdFactor[B] = instruction;
            return false;

        case LoadImageContributionFactorB: // Load image contribution factor for B.
            m_icf[B] = bits<0, 5>(instruction);
            return false;
        }
    }

    // Common instructions.
    switch(code)
    {
    case NoOperation:
    case LoadControlTableLineStartPointer: // Avoid going in the default case.
    case LoadDisplayLineStartPointer:
        break;

    case SignalScanLine: // Signal when scan reaches this line.
        return true;

    case LoadDisplayParameters: // Load display parameters.
        m_imageType[PLANE] = decodeImageType(bits<0, 1>(instruction));
        m_pixelRepeatFactor[PLANE] = 1 << (1 + bits<2, 3>(instruction));
        m_bps[PLANE] = bit<8>(instruction);
        break;

    case SetCLUTBank: // Set CLUT bank.
        m_clutBank = bits<0, 1>(instruction);
        break;

    default:
        std::cout << "Unknow DCP instruction 0x" << std::hex << instruction << std::endl;
    }

    return false;
}

template bool Renderer::ExecuteDCPInstruction<Renderer::A>(uint32_t instruction) noexcept;
template bool Renderer::ExecuteDCPInstruction<Renderer::B>(uint32_t instruction) noexcept;

} // namespace Video
