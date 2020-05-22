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
            DecodeBitmapLineA();
        }
        else if(GetFT12_1() == 2)
        {
            DecodeRunLengthLine(planeA, Planes::PlaneA, &memory[GetVSR1()], GetCM1());
        }
        else
        {
            DecodeMosaicLineA();
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
            DecodeBitmapLineB();
        }
        else if(GetFT12_2() == 2)
        {
            DecodeRunLengthLine(planeB, Planes::PlaneB, &memory[GetVSR2()], GetCM2());
        }
        else
        {
            DecodeMosaicLineB();
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

void MCD212::DecodeBitmapLineA()
{
    const uint8_t codingMethod = controlRegisters[ImageCodingMethod] & 0x00000F;
    uint8_t* data = &memory[GetVSR1()];
    uint8_t* pixels = planeA.GetData() + lineNumber * planeA.GetWidth() * 3;
    uint8_t index = 0;
    uint32_t previous = controlRegisters[DYUVAbsStartValueForPlaneA];

    for(uint16_t x = 0; x < planeA.GetWidth();)
    {
        if(codingMethod == DYUV)
        {
            DecodeDYUV(data[index++], &pixels[x * 3], previous);
            previous = 0;
            previous |= pixels[x * 3] << 16;
            previous |= pixels[x * 3 + 1] << 8;
            previous |= pixels[x++ * 3 + 2];
        }
        else if(codingMethod == CLUT4)
        {
            DecodeCLUTA(data[index] & 0xF0, &pixels[x++ * 3], codingMethod);
            DecodeCLUTA(data[index++] << 4, &pixels[x++ * 3], codingMethod);
        }
        else // CLUT
        {
            DecodeCLUTA(data[index++], &pixels[x++ * 3], codingMethod);
        }
    }
}

void MCD212::DecodeBitmapLineB()
{
    const uint8_t codingMethod = (controlRegisters[ImageCodingMethod] & 0x000F00) >> 8;
    uint8_t* dataA = &memory[GetVSR1()];
    uint8_t* dataB = &memory[GetVSR2()];
    uint8_t* pixels = planeB.GetData() + lineNumber * planeB.GetWidth() * 3;
    uint8_t index = 0;
    uint32_t previous = controlRegisters[DYUVAbsStartValueForPlaneB];

    for(uint16_t x = 0; x < planeA.GetWidth();)
    {
        if(codingMethod == DYUV)
        {
            DecodeDYUV(dataB[index++], &pixels[x * 3], previous);
            previous = 0;
            previous |= pixels[x * 3] << 16;
            previous |= pixels[x * 3 + 1] << 8;
            previous |= pixels[x++ * 3 + 2];
        }
        else if(codingMethod == RGB555)
        {
            uint16_t color  = dataA[index] << 8;
            color |= dataB[index++];
            DecodeRGB555(color, &pixels[x++ * 3]);
        }
        else if(codingMethod == CLUT4)
        {
            DecodeCLUTB(dataB[index] & 0xF0, &pixels[x++ * 3], codingMethod);
            DecodeCLUTB(dataB[index++] << 4, &pixels[x++ * 3], codingMethod);
        }
        else // CLUT
        {
            DecodeCLUTB(dataB[index++], &pixels[x++ * 3], codingMethod);
        }
    }
}

void MCD212::DecodeRunLengthLine(wxImage& plane, Planes channel, uint8_t* data, bool cm)
{
    uint16_t index = 0;
    uint8_t* pixels = plane.GetData() + lineNumber * plane.GetWidth() * 3;
    void (MCD212::*DecodeCLUT)(const uint8_t, uint8_t[3], const uint8_t) = (channel == Planes::PlaneA) ? &MCD212::DecodeCLUTA : &MCD212::DecodeCLUTB;

    for(int x = 0; x < plane.GetWidth();)
    {
        uint8_t format = data[index++];
        if(cm) // RL3
        {
            uint8_t color1 =  format & 0x70;
            uint8_t color2 = (format & 0x07) << 4;
            uint16_t count = 1;
            if(format & 0x80) // run of pixels pairs
                count = data[index++];

            if(count == 0)
                count = plane.GetWidth() - x;

            for(int i = 0; i < count; i++)
            {
                (this->*DecodeCLUT)(color1, &pixels[x++ * 3], CLUT4);
                (this->*DecodeCLUT)(color2, &pixels[x++ * 3], CLUT4);
            }
        }
        else // RL7
        {
            uint8_t color =  format & 0x7F;
            uint16_t count = 1;
            if(format & 0x80) // run of pixels pairs
                count = data[index++];

            if(count == 0)
                count = plane.GetWidth() - x;

            for(int i = 0; i < count; i++)
                (this->*DecodeCLUT)(color, &pixels[x++ * 3], CLUT7);
        }
    }
}

void MCD212::DecodeMosaicLineA()
{
    const uint8_t codingMethod = controlRegisters[ImageCodingMethod] & 0x00000F;
    uint8_t* data = &memory[GetVSR1()];
    uint8_t* pixels = planeA.GetData() + lineNumber * planeA.GetWidth() * 3;
    uint8_t index = 0;
    uint32_t previous = controlRegisters[DYUVAbsStartValueForPlaneA];

    for(uint16_t x = 0; x < planeA.GetWidth();)
    {
        if(codingMethod == DYUV)
        {
            DecodeDYUV(data[index++], &pixels[x * 3], previous);
            for(uint16_t i = 1; i < GetMF12_1(); i++)
                memcpy(&pixels[(i + x) * 3], &pixels[x * 3], 3);
            previous = 0;
            previous |= pixels[x * 3] << 16;
            previous |= pixels[x * 3 + 1] << 8;
            previous |= pixels[x++ * 3 + 2];
        }
        else if(codingMethod == CLUT4)
        {
            DecodeCLUTA(data[index] & 0xF0, &pixels[x * 3], codingMethod);
            DecodeCLUTA(data[index++] << 4, &pixels[(x+1) * 3], codingMethod);
            for(uint16_t i = 1; i < GetMF12_1(); i++)
                memcpy(&pixels[(i * 2 + x) * 3], &pixels[x * 3], 6);
            x += 2;
        }
        else // CLUT
        {
            DecodeCLUTA(data[index++], &pixels[x * 3], codingMethod);
            for(uint16_t i = 1; i < GetMF12_1(); i++)
                memcpy(&pixels[(i + x) * 3], &pixels[x * 3], 3);
            x++;
        }
    }
}

void MCD212::DecodeMosaicLineB()
{
    const uint8_t codingMethod = (controlRegisters[ImageCodingMethod] & 0x000F00) >> 8;
    uint8_t* dataA = &memory[GetVSR1()];
    uint8_t* dataB = &memory[GetVSR2()];
    uint8_t* pixels = planeB.GetData() + lineNumber * planeB.GetWidth() * 3;
    uint8_t index = 0;
    uint32_t previous = controlRegisters[DYUVAbsStartValueForPlaneB];

    for(uint16_t x = 0; x < planeA.GetWidth();)
    {
        if(codingMethod == DYUV)
        {
            DecodeDYUV(dataB[index++], &pixels[x * 3], previous);
            for(uint16_t i = 1; i < GetMF12_2(); i++)
                memcpy(&pixels[(i + x) * 3], &pixels[x * 3], 3);
            previous = 0;
            previous |= pixels[x * 3] << 16;
            previous |= pixels[x * 3 + 1] << 8;
            previous |= pixels[x++ * 3 + 2];
        }
        else if(codingMethod == RGB555)
        {
            uint16_t color  = dataA[index] << 8;
            color |= dataB[index];
            DecodeRGB555(color, &pixels[x++ * 3]);
            for(uint16_t i = 1; i < GetMF12_2(); i++)
                memcpy(&pixels[(i + x) * 3], &pixels[x * 3], 3);
            x++;
        }
        else if(codingMethod == CLUT4)
        {
            DecodeCLUTB(dataB[index] & 0xF0, &pixels[x * 3], codingMethod);
            DecodeCLUTB(dataB[index++] << 4, &pixels[(x+1) * 3], codingMethod);
            for(uint16_t i = 1; i < GetMF12_2(); i++)
                memcpy(&pixels[(i * 2 + x) * 3], &pixels[x * 3], 6);
            x += 2;
        }
        else // CLUT
        {
            DecodeCLUTB(dataB[index++], &pixels[x * 3], codingMethod);
            for(uint16_t i = 1; i < GetMF12_2(); i++)
                memcpy(&pixels[(i + x) * 3], &pixels[x * 3], 3);
            x++;
        }
    }
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
        LOG(out_display << "WARNING: wrong CLUT type in channel A: " << (int)CLUTType << std::endl)
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
        LOG(out_display << "WARNING: wrong CLUT type in channel B: " << (int)CLUTType << std::endl)
        addr = 0;
    }

    pixels[0] = CLUT[addr] >> 16;
    pixels[1] = CLUT[addr] >> 8;
    pixels[2] = CLUT[addr];
}
