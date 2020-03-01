#include "MCD212.hpp"

#define   SET_DA_BIT() internalRegisters[CSR1R] |= 0x80;
#define UNSET_DA_BIT() internalRegisters[CSR1R] &= 0x20;

void MCD212::DisplayLine()
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
            if(!planeB.Create(GetHorizontalResolution2(), GetVerticalResolution()))
            {
                wxMessageBox("Could not create plane B image (" + std::to_string(GetHorizontalResolution2()) + "x" + std::to_string(GetVerticalResolution()) + ")");
                return;
            }
        }

        DisplayLineA();
        DisplayLineB();
        if(controlRegisters[CursorControl] & 0x800000) // Cursor enable bit
            DrawCursor();
    }

    if(++lineNumber >= GetVerticalResolution())
    {
        UNSET_DA_BIT()
        if(GetDE())
        {
            DrawBackground();

            wxImage* screen = new wxImage(GetHorizontalResolution1(), GetVerticalResolution());
            // do planeA + planeB + cursor + background here
            app->mainFrame->gamePanel->RefreshScreen(backgroundPlane);
            delete screen;
        }
        totalFrameCount++;
        OnFrameCompleted();
        lineNumber = 0;
    }
}

void MCD212::DisplayLineA()
{
    if(GetFT12_1() <= 1)
    {
        DecodeBitmap(planeA, &memory[GetVSR1()], GetHorizontalResolution1(), GetCM1());
    }
    else if(GetFT12_1() == 2)
    {
        DecodeRunLength(planeA, &memory[GetVSR1()], GetHorizontalResolution1(), GetCM1());
    }
    else
    {
        DecodeMosaic(planeA, &memory[GetVSR1()], GetHorizontalResolution1(), GetCM1());
    }

    if(GetIC1() && GetDC1())
        ExecuteDCA1();

    if(GetIC1() && lineNumber >= GetVerticalResolution()-1)
        ExecuteICA1();
}

void MCD212::DisplayLineB()
{
    if(GetFT12_2() <= 1)
    {
        DecodeBitmap(planeB, &memory[GetVSR2()], GetHorizontalResolution2(), GetCM2());
    }
    else if(GetFT12_2() == 2)
    {
        DecodeRunLength(planeB, &memory[GetVSR2()], GetHorizontalResolution2(), GetCM2());
    }
    else
    {
        DecodeMosaic(planeB, &memory[GetVSR2()], GetHorizontalResolution2(), GetCM2());
    }

    if(GetIC2() && GetDC2())
        ExecuteDCA2();

    if(GetIC2() && lineNumber >= GetVerticalResolution()-1)
        ExecuteICA2();
}

void MCD212::DecodeBitmap(wxImage& plane, uint8_t* data, uint16_t width, bool cm)
{
    uint8_t* pixels = plane.GetData();
    uint8_t* alpha = plane.GetAlpha();
    uint32_t index = 0;
    uint32_t curPixel = 0;

    if(cm) // 4 bits per pixel
    {
        for(uint16_t i = 0; i < width; i++)
        {
        }
    }
    else // 8 bits per pixel
    {
    }
}

void MCD212::DecodeRunLength(wxImage& plane, uint8_t* data, uint16_t width, bool cm)
{
    uint16_t index = 0;

    for(int x = 0; x < width;)
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

void MCD212::DecodeMosaic(wxImage& plane, uint8_t* data, uint16_t width, bool cm)
{
}

uint32_t DecodeRGB555(uint16_t pixel) // to check
{
    uint8_t r = (pixel & 0x7C00) >> 7;
    uint8_t g = (pixel & 0x03E0) >> 2;
    uint8_t b = (pixel & 0x001F) << 3;
    uint32_t a = (pixel & 0x8000) ? 0xFF000000 : 0;
    return a | (r << 16) | (g << 8) | b;
}

void DecodeDYUV(uint16_t pixel, uint32_t startValue, uint8_t pixels[6])
{
    uint8_t dequantizer[16] = {0, 1, 4, 9, 16, 27, 44, 79, 128, 177, 212, 229, 240, 247, 252, 255};
    uint8_t y, u, v, dy1, du1, dv1, dy2, du2, dv2;

    y = (startValue & 0x00FF0000) >> 16;
    u = signExtend816((startValue & 0x0000FF00) >> 8);
    v = signExtend816(startValue & 0x000000FF);

    du1 = (pixel & 0xF000) >> 12;
    dy1 = (pixel & 0x0F00) >> 8;
    dv1 = (pixel & 0x00F0) >> 4;
    dy2 = (pixel & 0x000F);
    // interpolate du2 and dv2


}

uint32_t MCD212::DecodeCLUT(uint8_t pixel)
{
    uint8_t address = (controlRegisters[CLUTBank] & 0x00000003) << 6;
    return 0;
}

void MCD212::DrawBackground()
{
    uint8_t* data  = backgroundPlane.GetData();
    uint8_t* alpha = backgroundPlane.GetAlpha();
    *alpha  = (controlRegisters[BackdropColor] & 0x000008) ? 255 : 128;
    data[0] = (controlRegisters[BackdropColor] & 0x000004) ? 255 : 0;
    data[1] = (controlRegisters[BackdropColor] & 0x000002) ? 255 : 0;
    data[2] = (controlRegisters[BackdropColor] & 0x000001) ? 255 : 0;
}

void MCD212::DrawCursor()
{
    uint8_t yAddress = controlRegisters[CursorPattern] >> 16 & 0x0F;
    uint8_t* data = cursorPlane.GetData() + 3*yAddress*16;
    uint8_t* alpha = cursorPlane.GetAlpha() + yAddress*16;

    uint16_t mask = 1 << 15;
    for(uint8_t i = 0; i < 16; i++)
    {
        if(controlRegisters[CursorPattern] & mask)
        {
            alpha[i]  = (controlRegisters[CursorPattern] & 0x000008) ? 255 : 128;
            data[3*i] = (controlRegisters[CursorPattern] & 0x000004) ? 255 : 0;
            data[3*i + 1] = (controlRegisters[CursorPattern] & 0x000002) ? 255 : 0;
            data[3*i + 2] = (controlRegisters[CursorPattern] & 0x000001) ? 255 : 0;
        }
        else
            alpha[i] = 0;
        mask >>= 1;
    }
}
