#include "MCD212.hpp"

#include <wx/msgdlg.h>

#define   SET_DA_BIT() internalRegisters[CSR1R] |= 0x80;
#define UNSET_DA_BIT() internalRegisters[CSR1R] &= 0x20;

void MCD212::DrawLine()
{
    SET_DA_BIT()

    if(GetDE())
    {
        if(lineNumber == 0)
        {
            if(!planeA.Create(GetHorizontalResolution1(), GetVerticalResolution()))
            {
                wxMessageBox("Could not create plane A image (" + std::to_string(GetHorizontalResolution1()) + "x" + std::to_string(GetVerticalResolution()) + ")");
                return;
            }
            if(!planeA.HasAlpha())
                planeA.InitAlpha();

            if(!planeB.Create(GetHorizontalResolution2(), GetVerticalResolution()))
            {
                wxMessageBox("Could not create plane B image (" + std::to_string(GetHorizontalResolution2()) + "x" + std::to_string(GetVerticalResolution()) + ")");
                return;
            }
            if(!planeB.HasAlpha())
                planeB.InitAlpha();

            if(!backgroundPlane.Create(GetHorizontalResolution1(), GetVerticalResolution()))
            {
                wxMessageBox("Could not create background image (" + std::to_string(GetHorizontalResolution1()) + "x" + std::to_string(GetVerticalResolution()) + ")");
                return;
            }
            if(!backgroundPlane.HasAlpha())
                backgroundPlane.InitAlpha();
        }

        DrawLineA();
        DrawLineB();
        DrawBackground();
        if(controlRegisters[CursorControl] & 0x800000) // Cursor enable bit
            DrawCursor();
    }

    if(++lineNumber >= GetVerticalResolution())
    {
        UNSET_DA_BIT()
        if(GetDE())
        {
            wxImage screen(backgroundPlane.GetWidth(), backgroundPlane.GetHeight());
            screen.Paste(backgroundPlane, 0, 0);

            if(controlRegisters[PlaneOrder])
            {
                screen.Paste(planeA, 0, 0);
                screen.Paste(planeB, 0, 0);
            }
            else
            {
                screen.Paste(planeB, 0, 0);
                screen.Paste(planeA, 0, 0);
            }

            if(controlRegisters[CursorControl] & 0x800000) // Cursor enable bit
            {
                uint16_t x =  controlRegisters[CursorPosition] & 0x000003FF;
                uint16_t y = (controlRegisters[CursorPosition] & 0x003FF000) >> 12;
                screen.Paste(cursorPlane, y, x);
            }

            app->mainFrame->gamePanel->RefreshScreen(screen);
        }
        totalFrameCount++;
        OnFrameCompleted();
        lineNumber = 0;
    }
}

void MCD212::DrawLineA()
{
    if(controlRegisters[ImageCodingMethod] & 0x00000F) // plane on
    {
        if(GetFT12_1() <= 1)
        {
            DecodeBitmapLine(planeA, &memory[GetVSR1()], GetCM1());
        }
        else if(GetFT12_1() == 2)
        {
            DecodeRunLengthLine(planeA, &memory[GetVSR1()], GetCM1());
        }
        else
        {
            DecodeMosaicLine(planeA, &memory[GetVSR1()], GetCM1());
        }
    }

    if(GetIC1() && GetDC1())
        ExecuteDCA1();

    if(GetIC1() && lineNumber >= GetVerticalResolution()-1)
        ExecuteICA1();
}

void MCD212::DrawLineB()
{

    if(controlRegisters[ImageCodingMethod] & 0x000F00) // plane on
    {
        if(GetFT12_2() <= 1)
        {
            DecodeBitmapLine(planeB, &memory[GetVSR2()], GetCM2());
        }
        else if(GetFT12_2() == 2)
        {
            DecodeRunLengthLine(planeB, &memory[GetVSR2()], GetCM2());
        }
        else
        {
            DecodeMosaicLine(planeB, &memory[GetVSR2()], GetCM2());
        }
    }

    if(GetIC2() && GetDC2())
        ExecuteDCA2();

    if(GetIC2() && lineNumber >= GetVerticalResolution()-1)
        ExecuteICA2();
}

void MCD212::DrawBackground()
{
    uint8_t* data  = backgroundPlane.GetData() + 3*lineNumber;
    uint8_t* alpha = backgroundPlane.GetAlpha() + lineNumber;
    uint8_t A = (controlRegisters[BackdropColor] & 0x000008) ? 255 : 128;
    uint8_t R = (controlRegisters[BackdropColor] & 0x000004) ? 255 : 0;
    uint8_t G = (controlRegisters[BackdropColor] & 0x000002) ? 255 : 0;
    uint8_t B = (controlRegisters[BackdropColor] & 0x000001) ? 255 : 0;

    for(uint16_t i = 0; i < backgroundPlane.GetWidth(); i++)
    {
        alpha[i]    = A;
        data[3*i]   = R;
        data[3*i+1] = G;
        data[3*i+2] = B;
    }
}

void MCD212::DrawCursor()
{
    // check if Y position starts at 0 or 1 (assuming 0 in this code)
    uint16_t yPosition = (controlRegisters[CursorPosition] & 0x003FF000) >> 12;
    if(lineNumber < yPosition || lineNumber + 16 > yPosition)
        return;

    uint8_t yAddress = controlRegisters[CursorPattern] >> 16 & 0x0F;
    uint8_t* data = cursorPlane.GetData() + 3*yAddress*16;
    uint8_t* alpha = cursorPlane.GetAlpha() + yAddress*16;

    uint16_t mask = 1 << 15;
    for(uint8_t i = 0; i < 16; i++)
    {
        if(controlRegisters[CursorPattern] & mask)
        {
            alpha[i]      = (controlRegisters[CursorPattern] & 0x000008) ? 255 : 128;
            data[3*i]     = (controlRegisters[CursorPattern] & 0x000004) ? 255 : 0;
            data[3*i + 1] = (controlRegisters[CursorPattern] & 0x000002) ? 255 : 0;
            data[3*i + 2] = (controlRegisters[CursorPattern] & 0x000001) ? 255 : 0;
        }
        else
            alpha[i] = 0;
        mask >>= 1;
    }
}

void MCD212::DecodeBitmapLine(wxImage& plane, uint8_t* data, bool cm)
{
    uint8_t* pixels = plane.GetData();
    uint8_t* alpha = plane.GetAlpha();
    uint32_t index = 0;
    uint32_t curPixel = 0;

    if(cm) // 4 bits per pixel
    {
        for(uint16_t i = 0; i < plane.GetWidth(); i++)
        {
        }
    }
    else // 8 bits per pixel
    {
    }
}

void MCD212::DecodeRunLengthLine(wxImage& plane, uint8_t* data, bool cm)
{
    uint16_t index = 0;

    for(int x = 0; x < plane.GetWidth();)
    {
        uint8_t format = data[index++];
        if(format & 0x80) // multiple pixels
        {
            uint8_t num = data[index];
            if(cm) // 4 bits / pixel
            {
                uint8_t color1 = (format & 0x70) >> 4;
                uint8_t color2 = (format & 0x07);
            }
            else // 8 bits / pixels
            {
                uint8_t color = format & 0x7F;
            }
        }
        else // single pixel
        {
            if(cm) // 4 bits / pixel
            {
                uint8_t color1 = (format & 0x70) >> 4;
                uint8_t color2 = (format & 0x07);
            }
            else // 8 bits / pixels
            {
                uint8_t color = format & 0x7F;
            }
        }
    }
}

void MCD212::DecodeMosaicLine(wxImage& plane, uint8_t* data, bool cm)
{
}

uint8_t MCD212::DecodeRGB555(const uint16_t pixel, uint8_t pixels[3])
{
    pixels[0] = (pixel & 0x7C00) >> 7;
    pixels[1] = (pixel & 0x03E0) >> 2;
    pixels[2] = (pixel & 0x001F) << 3;
    return (pixel & 0x8000) ? 0xFF : 0;
}

void MCD212::DecodeDYUV(const uint16_t pixel, uint8_t pixels[6], const uint32_t previous)
{
    uint8_t y1, u1, v1, y2, u2, v2, py, pu, pv;
    u1 = (pixel & 0xF000) >> 12;
    y1 = (pixel & 0x0F00) >> 8;
    v1 = (pixel & 0x00F0) >> 4;
    y2 =  pixel & 0x000F;
    py = previous >> 16;
    pu = previous >> 8;
    pv = previous;

    y1 = (py + dequantizer[y1]) % 256;
    u2 = u1 = (pu + dequantizer[u1]) % 256; // u2 should be interpolated with the next u1
    v2 = v1 = (pv + dequantizer[v1]) % 256; // v2 should be interpolated with the next v1
    y2 = (y1 + dequantizer[y1]) % 256;

    pixels[0] = y1 + (v1 - 128) * 1.371; // R1
    pixels[2] = y1 + (u1 - 128) * 1.733; // B1
    pixels[1] = (y1 - 0.299 * pixels[0] - 0.114 * pixels[2]) / 0.587; // G1
    pixels[3] = y2 + (v2 - 128) * 1.371; // R2
    pixels[5] = y2 + (u2 - 128) * 1.733; // B2
    pixels[4] = (y2 - 0.299 * pixels[3] - 0.114 * pixels[5]) / 0.587; // G2
}

void MCD212::DecodeCLUTA(const uint8_t pixel, uint8_t pixels[3], const uint8_t CLUTType)
{
    uint8_t addr;
    if(CLUTType == CLUT8)
    {
        addr = pixel;
    }
    else if(CLUTType == CLUT7)
    {
        addr = (pixel & 0x7F);
    }
    else if(CLUTType == CLUT77)
    {
        addr = (pixel & 0x7F) + (controlRegisters[ImageCodingMethod] & 0x00400000 ? 128 : 0);
    }
    else if(CLUTType == CLUT4)
    {
        addr = (pixel >> 4);
    }
    else
    {
        LOG(out << "WARNING: wrong CLUT type in channel A: " << (int)CLUTType << std::endl)
        addr = 0;
    }

    pixels[0] = CLUT[addr] >> 16;
    pixels[1] = CLUT[addr] >> 8;
    pixels[2] = CLUT[addr];
}

void MCD212::DecodeCLUTB(const uint8_t pixel, uint8_t pixels[3], const uint8_t CLUTType)
{
    uint8_t addr;
    if(CLUTType == CLUT7)
    {
        addr = (pixel & 0x7F) + 128;
    }
    else if(CLUTType == CLUT4)
    {
        addr = (pixel >> 4) + 128;
    }
    else
    {
        LOG(out << "WARNING: wrong CLUT type in channel B: " << (int)CLUTType << std::endl)
        addr = 0;
    }

    pixels[0] = CLUT[addr] >> 16;
    pixels[1] = CLUT[addr] >> 8;
    pixels[2] = CLUT[addr];
}
