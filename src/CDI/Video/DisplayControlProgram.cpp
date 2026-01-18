#include "DisplayParameters.hpp"
#include "VideoCommon.hpp"
#include "../common/panic.hpp"
#include "../common/utils.hpp"

#include <print>

namespace Video
{

/** \brief V.4.5.1, figures V.43, V.44 and V.45. */
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

/** \brief Returns the lowest 24-bits that contains the DCP command. */
static constexpr uint32_t dcpExtractCommand(const uint32_t inst) noexcept
{
    return inst & 0x00FF'FFFFu;
}

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
static constexpr DisplayParameters::ImageType decodeImageType(const uint8_t type) noexcept
{
    if(type <= 1)
        return DisplayParameters::ImageType::Normal;

    if(type == 2)
        return DisplayParameters::ImageType::RunLength;

    panic("Unsupported Mosaic image type");
    return DisplayParameters::ImageType::Mosaic;
}

/** \brief Figure V.49. */
static constexpr DisplayParameters::BitsPerPixel decodeBitsPerPixel(const uint8_t bps) noexcept
{
    switch(bps)
    {
    case 0:
        return DisplayParameters::BitsPerPixel::Normal8;

    case 1:
        return DisplayParameters::BitsPerPixel::Double4;

    case 2:
        return DisplayParameters::BitsPerPixel::High8;

    default:
        panic("Invalid bits per pixel {}", bps);
    }
}

/** \brief Executes the given DCP intruction.
 * \tparam PLANE The plane executing the instruction.
 * \return true if a video interrupt has to be generated, false otherwise.
 *
 * If an unknown instruction is provided, nothing is done.
 * The following instructions are not handled by this function:
 * - 0x20 Load control table line start pointer
 * - 0x40 Load display line start pointer
 */
template<ImagePlane PLANE>
bool DisplayParameters::ExecuteDCPInstruction(const uint32_t instruction) noexcept
{
    const uint8_t code = instruction >> 24;

    if(code >= CLUTBank0 && code <= CLUTBank63) // CLUT bank.
    {
        // TODO: In theory CLUT loading should be checked based on the coding methods (5.5 CLUT update).
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

    // On a plane-specific instruction executed on a different plane, do nothing.
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
        m_bps[PLANE] = decodeBitsPerPixel(bits<8, 9>(instruction));
        break;

    case SelectImageCodingMethod: // Select image coding methods.
        if constexpr(PLANE == A)
        {
            m_clutSelectHigh = bit<22>(instruction);
            m_matteNumber = bit<19>(instruction);
            m_codingMethod[B] = decodeCodingMethod1(bits<8, 11>(instruction));
            m_codingMethod[A] = decodeCodingMethod0(bits<0, 3>(instruction));
            if(!isAllowedImageCodingCombination(m_codingMethod[A], m_codingMethod[B]))
                std::println("Invalid image coding combination {} {}",
                    static_cast<int>(m_codingMethod[A]), static_cast<int>(m_codingMethod[B]));
            // TODO: what to do with external video enabled?
            m_externalVideo = bit<18>(instruction);
        }
        return false;

    case LoadTransparencyControl: // Load transparency control information.
        if constexpr(PLANE == A)
        {
            m_mix = !bit<23>(instruction);
            m_transparencyControl[B] = bits<8, 11>(instruction);
            m_transparencyControl[A] = bits<0, 3>(instruction);
        }
        return false;

    case LoadPlaneOrder: // Load plane order.
        if constexpr(PLANE == A)
            m_planeOrder = bit<0>(instruction);
        return false;

    case SetCLUTBank: // Set CLUT bank.
        m_clutBank = bits<0, 1>(instruction);
        break;

    case LoadTransparentColorA: // Load transparent color for plane A.
        if constexpr(PLANE == A)
            m_transparentColorRgb[A] = dcpExtractCommand(instruction);
        return false;

    case LoadTransparentColorB: // Load transparent color for plane B.
        if constexpr(PLANE == B)
            m_transparentColorRgb[B] = dcpExtractCommand(instruction);
        return false;

    case LoadMaskColorA: // Load mask color for plane A.
        if constexpr(PLANE == A)
            m_maskColorRgb[A] = dcpExtractCommand(instruction);
        return false;

    case LoadMaskColorB: // Load mask color for plane B.
        if constexpr(PLANE == B)
            m_maskColorRgb[B] = dcpExtractCommand(instruction);
        return false;

    case LoadDYUVStartValueA: // Load DYUV start value for plane A.
        if constexpr(PLANE == A)
            m_dyuvInitialValue[A] = dcpExtractCommand(instruction);
        return false;

    case LoadDYUVStartValueB: // Load DYUV start value for plane B.
        if constexpr(PLANE == B)
            m_dyuvInitialValue[B] = dcpExtractCommand(instruction);
        return false;

    case LoadBackdropColor: // Load backdrop color.
        if constexpr(PLANE == A)
            m_backdropColor = bits<0, 3>(instruction);
        return false;

    case LoadMosaicFactorA: // Load mosaic pixel hold factor for A.
        if constexpr(PLANE == A)
        {
            m_holdEnabled[A] = bit<23>(instruction);
            m_holdFactor[A] = instruction;
        }
        return false;

    case LoadMosaicFactorB: // Load mosaic pixel hold factor for B.
        if constexpr(PLANE == B)
        {
            m_holdEnabled[B] = bit<23>(instruction);
            m_holdFactor[B] = instruction;
        }
        return false;

    case LoadImageContributionFactorA: // Load image contribution factor for A.
        if constexpr(PLANE == A)
            m_icf[A] = bits<0, 5>(instruction);
        return false;

    case LoadImageContributionFactorB: // Load image contribution factor for B.
        if constexpr(PLANE == B)
            m_icf[B] = bits<0, 5>(instruction);
        return false;

    default:
        std::println(stderr, "Unknow DCP instruction {:#X}", instruction);
    }

    return false;
}

template bool DisplayParameters::ExecuteDCPInstruction<A>(uint32_t instruction) noexcept;
template bool DisplayParameters::ExecuteDCPInstruction<B>(uint32_t instruction) noexcept;

} // namespace Video
