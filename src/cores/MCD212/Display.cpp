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
//            screen.Paste(backgroundPlane, 0, 0);

            if(controlRegisters[PlaneOrder])
            {
//                screen.Paste(planeA, 0, 0);
//                screen.Paste(planeB, 0, 0);
            }
            else
            {
//                screen.Paste(planeB, 0, 0);
//                screen.Paste(planeA, 0, 0);
            }

            if(controlRegisters[CursorControl] & 0x800000) // Cursor enable bit
            {
                uint16_t x = controlRegisters[CursorPosition] & 0x0003FF;
                uint16_t y = controlRegisters[CursorPosition] >> 12 & 0x0003FF;
//                screen.Paste(cursorPlane, y, x);
            }
            Video::SplitARGB(planeA, GetHorizontalResolution1() * GetVerticalResolution() * 4, nullptr, screen);
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
            DecodeRunLengthLine(&planeA[lineNumber * GetHorizontalResolution1() * 4], CLUT, &memory[GetVSR1()], GetCM1());
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
            DecodeRunLengthLine(&planeB[lineNumber * GetHorizontalResolution2() * 4], &CLUT[128], &memory[GetVSR2()], GetCM2());
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
    uint8_t* pixels  = backgroundPlane + GetHorizontalResolution1() * lineNumber * 4;
    const uint8_t A = (controlRegisters[BackdropColor] & 0x000008) ? 255 : 128;
    const uint8_t R = (controlRegisters[BackdropColor] & 0x000004) ? 255 : 0;
    const uint8_t G = (controlRegisters[BackdropColor] & 0x000002) ? 255 : 0;
    const uint8_t B = (controlRegisters[BackdropColor] & 0x000001) ? 255 : 0;

    for(uint16_t i = 0, j = 0; i < GetHorizontalResolution1(); i++)
    {
        pixels[j++] = A;
        pixels[j++] = R;
        pixels[j++] = G;
        pixels[j++] = B;
    }
}

void MCD212::DrawLineCursor()
{
    // check if Y position starts at 0 or 1 (assuming 0 in this code)
    uint16_t yPosition = controlRegisters[CursorPosition] >> 12 & 0x0003FF;
    if(lineNumber < yPosition || lineNumber + 16 > yPosition)
        return;

    uint8_t yAddress = controlRegisters[CursorPattern] >> 16 & 0x0F;
    uint8_t* pixels = cursorPlane + yAddress * 16 * 4;

    const uint8_t A = (controlRegisters[CursorControl] & 0x000008) ? 255 : 128;
    const uint8_t R = (controlRegisters[CursorControl] & 0x000004) ? 255 : 0;
    const uint8_t G = (controlRegisters[CursorControl] & 0x000002) ? 255 : 0;
    const uint8_t B = (controlRegisters[CursorControl] & 0x000001) ? 255 : 0;

    uint16_t mask = 1 << 15;
    for(uint8_t i = 0, j = 0; i < 16; i++)
    {
        if(controlRegisters[CursorPattern] & mask)
        {
            pixels[j++] = A;
            pixels[j++] = R;
            pixels[j++] = G;
            pixels[j++] = B;
        }
        else
        {
            pixels[j] = 0;
            j += 4;
        }
        mask >>= 1;
    }
}

void MCD212::DecodeBitmapLineA()
{
    const uint8_t codingMethod = controlRegisters[ImageCodingMethod] & 0x00000F;
    const uint16_t width = GetHorizontalResolution1();
    uint8_t* data = &memory[GetVSR1()];
    uint8_t* pixels = planeA + lineNumber * width * 4;
    uint16_t index = 0;
    uint32_t previous = controlRegisters[DYUVAbsStartValueForPlaneA];

    for(uint16_t x = 0; x < width;)
    {
        if(codingMethod == DYUV)
        {
            Video::DecodeDYUV(data[index++], &pixels[x * 4], previous); // TODO
            previous = 0;
            previous |= pixels[x * 4] << 16;
            previous |= pixels[x * 4 + 1] << 8;
            previous |= pixels[x++ * 4 + 2];
        }
        else if(codingMethod == CLUT4)
        {
            Video::DecodeCLUT((data[index] >> 4 & 0x0F), &pixels[x++ * 4 + 1], CLUT);
            Video::DecodeCLUT(data[index++] & 0x0F, &pixels[x++ * 4 + 1], CLUT);
        }
        else // CLUT
        {
            Video::DecodeCLUT(data[index++] + (codingMethod == CLUT77 && controlRegisters[ImageCodingMethod] & 0x400000) ? 128 : 0, &pixels[x++ * 4 + 1], CLUT);
        }
    }
}

void MCD212::DecodeBitmapLineB()
{
    const uint8_t codingMethod = (controlRegisters[ImageCodingMethod] & 0x000F00) >> 8;
    const uint16_t width = GetHorizontalResolution2();
    uint8_t* dataA = &memory[GetVSR1()];
    uint8_t* dataB = &memory[GetVSR2()];
    uint8_t* pixels = planeB + lineNumber * width * 4;
    uint16_t index = 0;
    uint32_t previous = controlRegisters[DYUVAbsStartValueForPlaneB];

    for(uint16_t x = 0; x < width;)
    {
        if(codingMethod == DYUV)
        {
            Video::DecodeDYUV(dataB[index++], &pixels[x * 4], previous); // TODO
            previous = 0;
            previous |= pixels[x * 4] << 16;
            previous |= pixels[x * 4 + 1] << 8;
            previous |= pixels[x++ * 4 + 2];
        }
        else if(codingMethod == RGB555)
        {
            uint16_t color  = dataA[index] << 8;
            color |= dataB[index++];
            Video::DecodeRGB555(color, &pixels[x++ * 4 + 1]);
        }
        else if(codingMethod == CLUT4)
        {
            Video::DecodeCLUT((dataB[index] >> 4 & 0x0F) + 128, &pixels[x++ * 4 + 1], CLUT);
            Video::DecodeCLUT((dataB[index++] & 0x0F) + 128, &pixels[x++ * 4 + 1], CLUT);
        }
        else // CLUT
        {
            Video::DecodeCLUT(dataB[index++] + 128, &pixels[x++ * 4 + 1], CLUT);
        }
    }
}

void MCD212::DecodeRunLengthLine(uint8_t* line, const uint32_t* CLUTTable, const uint8_t* data, const bool cm)
{
    const uint16_t width = GetHorizontalResolution1();

    for(int x = 0; x < width;)
    {
        const uint8_t format = *data++;
        if(cm) // RL3
        {
            const uint8_t color1 = format >> 4 & 0x07;
            const uint8_t color2 = format & 0x07;
            uint16_t count = 1;
            if(format & 0x80) // run of pixels pairs
            {
                count = *data++;
                if(count == 0)
                    count = width - x;
            }

            for(int i = 0; i < count; i++)
            {
                line[x * 4] = 0xFF;
                Video::DecodeCLUT(color1, &line[x++ * 4 + 1], CLUTTable);
                line[x * 4] = 0xFF;
                Video::DecodeCLUT(color2, &line[x++ * 4 + 1], CLUTTable);
            }
        }
        else // RL7
        {
            const uint8_t color = format & 0x7F;
            uint16_t count = 1;
            if(format & 0x80) // run of single pixels
            {
                count = *data++;
                if(count == 0)
                    count = width - x;
            }

            for(int i = 0; i < count; i++)
            {
                line[x * 4] = 0xFF;
                Video::DecodeCLUT(color, &line[x++ * 4 + 1], CLUTTable);
            }
        }
    }
}

void MCD212::DecodeMosaicLineA() // TODO
{
    const uint8_t codingMethod = controlRegisters[ImageCodingMethod] & 0x00000F;
    const uint16_t width = GetHorizontalResolution1();
    uint8_t* data = &memory[GetVSR1()];
    uint8_t* pixels = planeA + lineNumber * width * 4;
    uint16_t index = 0;
    uint32_t previous = controlRegisters[DYUVAbsStartValueForPlaneA];

    for(uint16_t x = 0; x < width;)
    {
        if(codingMethod == DYUV)
        {
            Video::DecodeDYUV(data[index++], &pixels[x * 4], previous);
            for(uint16_t i = 1; i < GetMF12_1(); i++)
                memcpy(&pixels[(i + x) * 4], &pixels[x * 4], 4);
            previous = 0;
            previous |= pixels[x * 4] << 16;
            previous |= pixels[x * 4 + 1] << 8;
            previous |= pixels[x * 4 + 2];
            x += GetMF12_1();
        }
        else if(codingMethod == CLUT4)
        {
            Video::DecodeCLUT((data[index] >> 4 & 0x0F), &pixels[x * 4], CLUT);
            Video::DecodeCLUT(data[index++] & 0x0F, &pixels[(x+1) * 4], CLUT);
            for(uint16_t i = 1; i < GetMF12_1(); i++)
                memcpy(&pixels[(i * 2 + x) * 4], &pixels[x * 4], 8);
            x += GetMF12_1() * 2;
        }
        else // CLUT
        {
            Video::DecodeCLUT(data[index++] + (codingMethod == CLUT77 && controlRegisters[ImageCodingMethod] & 0x400000) ? 128 : 0, &pixels[x * 4], CLUT);
            for(uint16_t i = 1; i < GetMF12_1(); i++)
                memcpy(&pixels[(i + x) * 4], &pixels[x * 4], 4);
            x += GetMF12_1();
        }
    }
}

void MCD212::DecodeMosaicLineB() // TODO
{
    const uint8_t codingMethod = (controlRegisters[ImageCodingMethod] & 0x000F00) >> 8;
    const uint16_t width = GetHorizontalResolution2();
    uint8_t* dataA = &memory[GetVSR1()];
    uint8_t* dataB = &memory[GetVSR2()];
    uint8_t* pixels = planeB + lineNumber * width * 4;
    uint16_t index = 0;
    uint32_t previous = controlRegisters[DYUVAbsStartValueForPlaneB];

    for(uint16_t x = 0; x < width;)
    {
        if(codingMethod == DYUV)
        {
            Video::DecodeDYUV(dataB[index++], &pixels[x * 4], previous);
            for(uint16_t i = 1; i < GetMF12_2(); i++)
                memcpy(&pixels[(i + x) * 4], &pixels[x * 4], 4);
            previous = 0;
            previous |= pixels[x * 4] << 16;
            previous |= pixels[x * 4 + 1] << 8;
            previous |= pixels[x * 4 + 2];
            x += GetMF12_2();
        }
        else if(codingMethod == RGB555)
        {
            uint16_t color  = dataA[index] << 8;
            color |= dataB[index];
            Video::DecodeRGB555(color, &pixels[x++ * 4]);
            for(uint16_t i = 1; i < GetMF12_2(); i++)
                memcpy(&pixels[(i + x) * 4], &pixels[x * 4], 4);
            x += GetMF12_2();
        }
        else if(codingMethod == CLUT4)
        {
            Video::DecodeCLUT((dataB[index] >> 4) & 0x0F, &pixels[x * 4], CLUT);
            Video::DecodeCLUT(dataB[index++] & 0x0F, &pixels[(x+1) * 4], CLUT);
            for(uint16_t i = 1; i < GetMF12_2(); i++)
                memcpy(&pixels[(i * 2 + x) * 4], &pixels[x * 4], 8);
            x += GetMF12_2() * 2;
        }
        else // CLUT
        {
            Video::DecodeCLUT(dataB[index++] + 128, &pixels[x * 4], CLUT);
            for(uint16_t i = 1; i < GetMF12_2(); i++)
                memcpy(&pixels[(i + x) * 4], &pixels[x * 4], 4);
            x += GetMF12_2();
        }
    }
}

#undef   SET_DA_BIT
#undef UNSET_DA_BIT
