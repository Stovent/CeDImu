#include "MCD212.hpp"
#include "../../Boards/Board.hpp"
#include "../../utils.hpp"
#include "../../common/Video.hpp"

#include <wx/msgdlg.h>

#define   SET_DA_BIT() internalRegisters[MCSR1R] |= 0x80;
#define UNSET_DA_BIT() internalRegisters[MCSR1R] &= 0x20;

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

        DrawLinePlaneA();
        DrawLinePlaneB();
        DrawLineBackground();
        if(controlRegisters[CursorControl] & 0x800000) // Cursor enable bit
            DrawLineCursor();
    }

    if(++lineNumber >= GetVerticalResolution())
    {
        UNSET_DA_BIT()
        if(GetDE())
        {
            if(!screen.Create(backgroundPlane.GetWidth(), backgroundPlane.GetHeight()))
            {
                wxMessageBox("Could not create screen (" + std::to_string(GetHorizontalResolution1()) + "x" + std::to_string(GetVerticalResolution()) + ")");
                return;
            }

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
        }

        if(OnFrameCompleted)
            OnFrameCompleted();

        lineNumber = 0;
        totalFrameCount++;
        if(stopOnNextFrame)
        {
            board->cpu.Stop(false);
            stopOnNextFrame = false;
        }
    }
}

void MCD212::DrawLinePlaneA()
{
    if(controlRegisters[ImageCodingMethod] & 0x00000F) // plane on
    {
        if(GetFT12_1() <= 1)
        {
            DecodeBitmapLineA();
        }
        else if(GetFT12_1() == 2)
        {
            DecodeRunLengthLine(planeA, &Video::DecodeCLUT, &memory[GetVSR1()], GetCM1());
        }
        else
        {
            DecodeMosaicLineA();
        }
    }

    if(GetIC1() && GetDC1())
    {
        DCA1.clear();
        ExecuteDCA1();
    }

    if(GetIC1() && lineNumber >= GetVerticalResolution()-1)
    {
        ICA1.clear();
        ExecuteICA1();
    }
}

void MCD212::DrawLinePlaneB()
{

    if(controlRegisters[ImageCodingMethod] & 0x000F00) // plane on
    {
        if(GetFT12_2() <= 1)
        {
            DecodeBitmapLineB();
        }
        else if(GetFT12_2() == 2)
        {
            DecodeRunLengthLine(planeB, &Video::DecodeCLUT, &memory[GetVSR2()], GetCM2()); // TODO: remove the necessity to send the CLUT decoder function
        }
        else
        {
            DecodeMosaicLineB();
        }
    }

    if(GetIC2() && GetDC2())
    {
        DCA2.clear();
        ExecuteDCA2();
    }

    if(GetIC2() && lineNumber >= GetVerticalResolution()-1)
    {
        ICA2.clear();
        ExecuteICA2();
    }
}

void MCD212::DrawLineBackground()
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

void MCD212::DrawLineCursor()
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
    uint16_t index = 0;
    uint32_t previous = controlRegisters[DYUVAbsStartValueForPlaneA];

    for(uint16_t x = 0; x < planeA.GetWidth();)
    {
        if(codingMethod == DYUV)
        {
            Video::DecodeDYUV(data[index++], &pixels[x * 3], previous);
            previous = 0;
            previous |= pixels[x * 3] << 16;
            previous |= pixels[x * 3 + 1] << 8;
            previous |= pixels[x++ * 3 + 2];
        }
        else if(codingMethod == CLUT4)
        {
            Video::DecodeCLUT(data[index] >> 4 & 0x0F), &pixels[x++ * 3], CLUT);
            Video::DecodeCLUT(data[index++] & 0x0F, &pixels[x++ * 3], CLUT);
        }
        else // CLUT
        {
            Video::DecodeCLUT(data[index++], &pixels[x++ * 3], CLUT);
        }
    }
}

void MCD212::DecodeBitmapLineB()
{
    const uint8_t codingMethod = (controlRegisters[ImageCodingMethod] & 0x000F00) >> 8;
    uint8_t* dataA = &memory[GetVSR1()];
    uint8_t* dataB = &memory[GetVSR2()];
    uint8_t* pixels = planeB.GetData() + lineNumber * planeB.GetWidth() * 3;
    uint16_t index = 0;
    uint32_t previous = controlRegisters[DYUVAbsStartValueForPlaneB];

    for(uint16_t x = 0; x < planeA.GetWidth();)
    {
        if(codingMethod == DYUV)
        {
            Video::DecodeDYUV(dataB[index++], &pixels[x * 3], previous);
            previous = 0;
            previous |= pixels[x * 3] << 16;
            previous |= pixels[x * 3 + 1] << 8;
            previous |= pixels[x++ * 3 + 2];
        }
        else if(codingMethod == RGB555)
        {
            uint16_t color  = dataA[index] << 8;
            color |= dataB[index++];
            Video::DecodeRGB555(color, &pixels[x++ * 3]);
        }
        else if(codingMethod == CLUT4)
        {
            Video::DecodeCLUT((dataB[index] >> 4 & 0x0F) + 128, &pixels[x++ * 3], CLUT);
            Video::DecodeCLUT((dataB[index++] & 0x0F) + 128, &pixels[x++ * 3], CLUT);
        }
        else // CLUT
        {
            Video::DecodeCLUT(dataB[index++] + 128, &pixels[x++ * 3], CLUT);
        }
    }
}

void MCD212::DecodeRunLengthLine(wxImage& plane, void (*CLUTDecoder)(const uint8_t, uint8_t[3], const uint32_t*), uint8_t* data, bool cm)
{
    uint16_t index = 0;
    uint8_t* pixels = plane.GetData() + lineNumber * plane.GetWidth() * 3;

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
                CLUTDecoder(color1, &pixels[x++ * 3], CLUT);
                CLUTDecoder(color2, &pixels[x++ * 3], CLUT);
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
                CLUTDecoder(color, &pixels[x++ * 3], CLUT);
        }
    }
}

void MCD212::DecodeMosaicLineA()
{
    const uint8_t codingMethod = controlRegisters[ImageCodingMethod] & 0x00000F;
    uint8_t* data = &memory[GetVSR1()];
    uint8_t* pixels = planeA.GetData() + lineNumber * planeA.GetWidth() * 3;
    uint16_t index = 0;
    uint32_t previous = controlRegisters[DYUVAbsStartValueForPlaneA];

    for(uint16_t x = 0; x < planeA.GetWidth();)
    {
        if(codingMethod == DYUV)
        {
            Video::DecodeDYUV(data[index++], &pixels[x * 3], previous);
            for(uint16_t i = 1; i < GetMF12_1(); i++)
                memcpy(&pixels[(i + x) * 3], &pixels[x * 3], 3);
            previous = 0;
            previous |= pixels[x * 3] << 16;
            previous |= pixels[x * 3 + 1] << 8;
            previous |= pixels[x * 3 + 2];
            x += GetMF12_1();
        }
        else if(codingMethod == CLUT4)
        {
            Video::DecodeCLUT((data[index] >> 4 & 0x0F), &pixels[x * 3], CLUT);
            Video::DecodeCLUT(data[index++] & 0x0F, &pixels[(x+1) * 3], CLUT);
            for(uint16_t i = 1; i < GetMF12_1(); i++)
                memcpy(&pixels[(i * 2 + x) * 3], &pixels[x * 3], 6);
            x += GetMF12_1() * 2;
        }
        else // CLUT
        {
            Video::DecodeCLUT(data[index++], &pixels[x * 3], CLUT);
            for(uint16_t i = 1; i < GetMF12_1(); i++)
                memcpy(&pixels[(i + x) * 3], &pixels[x * 3], 3);
            x += GetMF12_1();
        }
    }
}

void MCD212::DecodeMosaicLineB()
{
    const uint8_t codingMethod = (controlRegisters[ImageCodingMethod] & 0x000F00) >> 8;
    uint8_t* dataA = &memory[GetVSR1()];
    uint8_t* dataB = &memory[GetVSR2()];
    uint8_t* pixels = planeB.GetData() + lineNumber * planeB.GetWidth() * 3;
    uint16_t index = 0;
    uint32_t previous = controlRegisters[DYUVAbsStartValueForPlaneB];

    for(uint16_t x = 0; x < planeA.GetWidth();)
    {
        if(codingMethod == DYUV)
        {
            Video::DecodeDYUV(dataB[index++], &pixels[x * 3], previous);
            for(uint16_t i = 1; i < GetMF12_2(); i++)
                memcpy(&pixels[(i + x) * 3], &pixels[x * 3], 3);
            previous = 0;
            previous |= pixels[x * 3] << 16;
            previous |= pixels[x * 3 + 1] << 8;
            previous |= pixels[x * 3 + 2];
            x += GetMF12_2();
        }
        else if(codingMethod == RGB555)
        {
            uint16_t color  = dataA[index] << 8;
            color |= dataB[index];
            Video::DecodeRGB555(color, &pixels[x++ * 3]);
            for(uint16_t i = 1; i < GetMF12_2(); i++)
                memcpy(&pixels[(i + x) * 3], &pixels[x * 3], 3);
            x += GetMF12_2();
        }
        else if(codingMethod == CLUT4)
        {
            Video::DecodeCLUT((dataB[index] >> 4) & 0x0F, &pixels[x * 3], CLUT);
            Video::DecodeCLUT(dataB[index++] & 0x0F, &pixels[(x+1) * 3], CLUT);
            for(uint16_t i = 1; i < GetMF12_2(); i++)
                memcpy(&pixels[(i * 2 + x) * 3], &pixels[x * 3], 6);
            x += GetMF12_2() * 2;
        }
        else // CLUT
        {
            Video::DecodeCLUT(dataB[index++] + 128, &pixels[x * 3], CLUT);
            for(uint16_t i = 1; i < GetMF12_2(); i++)
                memcpy(&pixels[(i + x) * 3], &pixels[x * 3], 3);
            x += GetMF12_2();
        }
    }
}

#undef   SET_DA_BIT
#undef UNSET_DA_BIT
