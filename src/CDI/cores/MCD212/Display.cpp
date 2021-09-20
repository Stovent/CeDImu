#include "MCD212.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"
#include "../../common/Video.hpp"

#include <cstring>

#define   SET_DA_BIT() registerCSR1R |= 0x80;
#define UNSET_DA_BIT() registerCSR1R &= 0x20;
#define   SET_PA_BIT() registerCSR1R |= 0x20;
#define UNSET_PA_BIT() registerCSR1R &= 0x80;

void MCD212::ExecuteVideoLine()
{
    if(++verticalLines <= GetVerticalRetraceLines())
    {
        if(verticalLines == 1 && GetDE()) // TODO: how to do this on every vertical line?
        {
            if(GetIC1())
            {
                ExecuteICA1();
            }

            if(GetIC2())
            {
                ExecuteICA2();
            }
        }
        return;
    }

    SET_DA_BIT()

    if(lineNumber == 0)
    {
        screen.width = backgroundPlane.width = planeA.width = GetHorizontalResolution1();
        planeB.width = GetHorizontalResolution2();
        screen.height = backgroundPlane.height = planeB.height = planeA.height = GetVerticalResolution();
    }

    if(GetSM() && !isEven(lineNumber)) // not even because my line count starts at 0.
        UNSET_PA_BIT()
    else
        SET_PA_BIT()

    if(GetDE())
    {
        DrawLinePlaneA();
        DrawLinePlaneB();
        DrawLineBackground();
        if(controlRegisters[CursorControl] & 0x800000) // Cursor enable bit
            DrawLineCursor();

        if(GetIC1() && GetDC1())
        {
            ExecuteDCA1();
        }

        if(GetIC2() && GetDC2())
        {
            ExecuteDCA2();
        }
    }

    lineNumber++;
    if(verticalLines >= GetTotalVerticalLines())
    {
        if(GetDE())
        {
            Video::splitARGB(backgroundPlane.data(), backgroundPlane.width * backgroundPlane.height * 4, nullptr, screen.data());

            if(controlRegisters[PlaneOrder] & 1)
            {
                Video::paste(screen.data(), screen.width, screen.height, planeA.data(), planeA.width, planeA.height);
                Video::paste(screen.data(), screen.width, screen.height, planeB.data(), planeB.width, planeB.height);
            }
            else
            {
                Video::paste(screen.data(), screen.width, screen.height, planeB.data(), planeB.width, planeB.height);
                Video::paste(screen.data(), screen.width, screen.height, planeA.data(), planeA.width, planeA.height);
            }

            if(controlRegisters[CursorControl] & 0x800000) // Cursor enable bit
            {
                const uint16_t x = (controlRegisters[CursorPosition] & 0x0003FF) >> 1; // TODO: address is in double resolution mode
                const uint16_t y = controlRegisters[CursorPosition] >> 12 & 0x0003FF;
                Video::paste(screen.data(), screen.width, screen.height, cursorPlane.data(), CURSOR_WIDTH, CURSOR_HEIGHT, x, y);
            }
        }

        UNSET_DA_BIT()
        totalFrameCount++;
        lineNumber = 0;
        verticalLines = 0;

        cdi.callbacks.OnFrameCompleted(screen);
    }
}

void MCD212::DrawLinePlaneA()
{
    uint16_t bytes = 0;
    if(controlRegisters[ImageCodingMethod] & 0x00000F) // plane on
    {
        const uint8_t fileType = GetFT12_1();
        if(fileType <= 1)
        {
            const uint8_t codingMethod = controlRegisters[ImageCodingMethod] & 0x00000F;
            bytes = Video::decodeBitmapLine(&planeA[lineNumber * planeA.width * 4], planeA.width, nullptr, &memory[GetVSR1()], codingMethod == CLUT77 && controlRegisters[ImageCodingMethod] & 0x400000 ? &CLUT[128] : CLUT.data(), controlRegisters[DYUVAbsStartValueForPlaneA], codingMethod);

            if(codingMethod == CLUT4 || codingMethod == CLUT7 || codingMethod == CLUT77 || codingMethod == CLUT8)
                HandleCLUTTransparency(&planeA[lineNumber * planeA.width * 4], planeA.width, controlRegisters[TransparencyControl] & 0xF, controlRegisters[TransparentColorForPlaneA]);
        }
        else if(fileType == 2)
        {
            bytes = Video::decodeRunLengthLine(&planeA[lineNumber * planeA.width * 4], planeA.width, &memory[GetVSR1()], CLUT.data(), GetCM1());
            HandleCLUTTransparency(&planeA[lineNumber * planeA.width * 4], planeA.width, controlRegisters[TransparencyControl] & 0xF, controlRegisters[TransparentColorForPlaneA]);
        }
        else
        {
            DecodeMosaicLineA();
        }
    }
    SetVSR1(GetVSR1() + bytes);
}

void MCD212::DrawLinePlaneB()
{
    uint16_t bytes = 0;
    if(controlRegisters[ImageCodingMethod] & 0x000F00) // plane on
    {
        const uint8_t fileType = GetFT12_2();
        if(fileType <= 1)
        {
            const uint8_t codingMethod = controlRegisters[ImageCodingMethod] >> 8 & 0x00000F;
            bytes = Video::decodeBitmapLine(&planeB[lineNumber * planeB.width * 4], planeB.width, &memory[GetVSR1()], &memory[GetVSR2()], &CLUT[128], controlRegisters[DYUVAbsStartValueForPlaneB], codingMethod);

            if(codingMethod == CLUT4 || codingMethod == CLUT7)
                HandleCLUTTransparency(&planeB[lineNumber * planeB.width * 4], planeB.width, controlRegisters[TransparencyControl] >> 8 & 0xF, controlRegisters[TransparentColorForPlaneB]);
        }
        else if(fileType == 2)
        {
            bytes = Video::decodeRunLengthLine(&planeB[lineNumber * planeB.width * 4], planeB.width, &memory[GetVSR2()], &CLUT[128], GetCM2());
            HandleCLUTTransparency(&planeB[lineNumber * planeB.width * 4], planeB.width, controlRegisters[TransparencyControl] >> 8 & 0xF, controlRegisters[TransparentColorForPlaneB]);
        }
        else
        {
            DecodeMosaicLineB();
        }
    }
    SetVSR2(GetVSR2() + bytes);
}

void MCD212::DrawLineBackground()
{
    uint8_t* pixels  = &backgroundPlane[backgroundPlane.width * lineNumber * 4];
    const uint8_t A = (controlRegisters[BackdropColor] & 0x000008) ? 255 : 128;
    const uint8_t R = (controlRegisters[BackdropColor] & 0x000004) ? 255 : 0;
    const uint8_t G = (controlRegisters[BackdropColor] & 0x000002) ? 255 : 0;
    const uint8_t B = (controlRegisters[BackdropColor] & 0x000001) ? 255 : 0;

    for(uint16_t i = 0, j = 0; i < backgroundPlane.width; i++)
    {
        pixels[j++] = A;
        pixels[j++] = R;
        pixels[j++] = G;
        pixels[j++] = B;
    }
}

void MCD212::DrawLineCursor()
{
    const uint16_t yPosition = controlRegisters[CursorPosition] >> 12 & 0x0003FF;
    if(lineNumber < yPosition || lineNumber >= yPosition + 16)
        return;

    const uint8_t yAddress = lineNumber - yPosition;
    uint8_t* pixels = &cursorPlane[yAddress * 16 * 4];

    const uint8_t A = (controlRegisters[CursorControl] & 0x000008) ? 255 : 128;
    const uint8_t R = (controlRegisters[CursorControl] & 0x000004) ? 255 : 0;
    const uint8_t G = (controlRegisters[CursorControl] & 0x000002) ? 255 : 0;
    const uint8_t B = (controlRegisters[CursorControl] & 0x000001) ? 255 : 0;

    uint16_t mask = 1 << 15;
    for(uint8_t i = 0, j = 0; i < 16; i++)
    {
        if(cursorPatterns[yAddress] & mask)
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

void MCD212::HandleCLUTTransparency(uint8_t* pixels, const uint16_t width, const uint32_t control, const uint32_t color)
{
    switch(control)
    {
    case 0: // Always
        for(uint16_t i = 0; i < width; i++, pixels += 4)
            *pixels = 0;
        break;

    case 1: // Color Key = True
        for(uint16_t i = 0; i < width; i++, pixels += 4)
            if(*(pixels + 1) == (color >> 16 & 0xFF) && *(pixels + 2) == (color >> 8 & 0xFF) && *(pixels + 3) == (color & 0xFF))
                *pixels = 0;
        break;

    case 9: // Color Key = False
        for(uint16_t i = 0; i < width; i++, pixels += 4)
            if(!(*(pixels + 1) == (color >> 16 & 0xFF) && *(pixels + 2) == (color >> 8 & 0xFF) && *(pixels + 3) == (color & 0xFF)))
                *pixels = 0;
        break;
    }
}

void MCD212::DecodeMosaicLineA() // TODO
{
    const uint8_t codingMethod = controlRegisters[ImageCodingMethod] & 0x00000F;
    uint8_t* data = &memory[GetVSR1()];
    uint8_t* pixels = &planeA[lineNumber * planeA.width * 4];
    uint16_t index = 0;
    uint32_t previous = controlRegisters[DYUVAbsStartValueForPlaneA];

    for(uint16_t x = 0; x < planeA.width;)
    {
        if(codingMethod == DYUV)
        {
            Video::decodeDYUV(data[index++], &pixels[x * 4], previous);
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
            Video::decodeCLUT((data[index] >> 4 & 0x0F), &pixels[x * 4], CLUT.data());
            Video::decodeCLUT(data[index++] & 0x0F, &pixels[(x+1) * 4], CLUT.data());
            for(uint16_t i = 1; i < GetMF12_1(); i++)
                memcpy(&pixels[(i * 2 + x) * 4], &pixels[x * 4], 8);
            x += GetMF12_1() * 2;
        }
        else // CLUT
        {
            Video::decodeCLUT(data[index++] + (codingMethod == CLUT77 && controlRegisters[ImageCodingMethod] & 0x400000) ? 128 : 0, &pixels[x * 4], CLUT.data());
            for(uint16_t i = 1; i < GetMF12_1(); i++)
                memcpy(&pixels[(i + x) * 4], &pixels[x * 4], 4);
            x += GetMF12_1();
        }
    }
}

void MCD212::DecodeMosaicLineB() // TODO
{
    const uint8_t codingMethod = (controlRegisters[ImageCodingMethod] & 0x000F00) >> 8;
    uint8_t* dataA = &memory[GetVSR1()];
    uint8_t* dataB = &memory[GetVSR2()];
    uint8_t* pixels = &planeB[lineNumber * planeB.width * 4];
    uint16_t index = 0;
    uint32_t previous = controlRegisters[DYUVAbsStartValueForPlaneB];

    for(uint16_t x = 0; x < planeB.width;)
    {
        if(codingMethod == DYUV)
        {
            Video::decodeDYUV(dataB[index++], &pixels[x * 4], previous);
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
            Video::decodeRGB555(color, &pixels[x++ * 4]);
            for(uint16_t i = 1; i < GetMF12_2(); i++)
                memcpy(&pixels[(i + x) * 4], &pixels[x * 4], 4);
            x += GetMF12_2();
        }
        else if(codingMethod == CLUT4)
        {
            Video::decodeCLUT((dataB[index] >> 4) & 0x0F, &pixels[x * 4], CLUT.data());
            Video::decodeCLUT(dataB[index++] & 0x0F, &pixels[(x+1) * 4], CLUT.data());
            for(uint16_t i = 1; i < GetMF12_2(); i++)
                memcpy(&pixels[(i * 2 + x) * 4], &pixels[x * 4], 8);
            x += GetMF12_2() * 2;
        }
        else // CLUT
        {
            Video::decodeCLUT(dataB[index++] + 128, &pixels[x * 4], CLUT.data());
            for(uint16_t i = 1; i < GetMF12_2(); i++)
                memcpy(&pixels[(i + x) * 4], &pixels[x * 4], 4);
            x += GetMF12_2();
        }
    }
}
