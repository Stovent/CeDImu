#include "MCD212.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"
#include "../../common/Video.hpp"

#include <cstring>

#define   SET_DA_BIT() registerCSR1R |= 0x80;
#define UNSET_DA_BIT() registerCSR1R &= 0x20;
#define   SET_PA_BIT() registerCSR1R |= 0x20;
#define UNSET_PA_BIT() registerCSR1R &= 0x80;

static constexpr Video::ImageCodingMethod ICM_LUT_A[16] = {
    ICM(OFF),    ICM(CLUT8), ICM(OFF), ICM(CLUT7),
    ICM(CLUT77), ICM(DYUV),  ICM(OFF), ICM(OFF),
    ICM(OFF),    ICM(OFF),   ICM(OFF), ICM(CLUT4),
    ICM(OFF),    ICM(OFF),   ICM(OFF), ICM(OFF),
};

static constexpr Video::ImageCodingMethod ICM_LUT_B[16] = {
    ICM(OFF),    ICM(RGB555), ICM(OFF), ICM(CLUT7),
    ICM(CLUT77), ICM(DYUV),   ICM(OFF), ICM(OFF),
    ICM(OFF),    ICM(OFF),    ICM(OFF), ICM(CLUT4),
    ICM(OFF),    ICM(OFF),    ICM(OFF), ICM(OFF),
};

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
        screen.width = planeA.width = GetHorizontalResolution1();
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
        OverlayMix();

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
            const Video::ImageCodingMethod codingMethod = ICM_LUT_A[controlRegisters[ImageCodingMethod] & 0x00000F];
            const uint32_t* clut = codingMethod == ICM(CLUT77) && controlRegisters[ImageCodingMethod] & 0x400000 ? &CLUT[128] : CLUT.data();
            bytes = Video::decodeBitmapLine(&planeA[lineNumber * planeA.width * 4], planeA.width, nullptr, &memory[GetVSR1()], clut, controlRegisters[DYUVAbsStartValueForPlaneA], codingMethod);
        }
        else if(fileType == 2)
        {
            bytes = Video::decodeRunLengthLine(&planeA[lineNumber * planeA.width * 4], planeA.width, &memory[GetVSR1()], CLUT.data(), GetCM1());
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
            const Video::ImageCodingMethod codingMethod = ICM_LUT_B[controlRegisters[ImageCodingMethod] >> 8 & 0x00000F];
            bytes = Video::decodeBitmapLine(&planeB[lineNumber * planeB.width * 4], planeB.width, &memory[GetVSR1()], &memory[GetVSR2()], &CLUT[128], controlRegisters[DYUVAbsStartValueForPlaneB], codingMethod);
        }
        else if(fileType == 2)
        {
            bytes = Video::decodeRunLengthLine(&planeB[lineNumber * planeB.width * 4], planeB.width, &memory[GetVSR2()], &CLUT[128], GetCM2());
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
    // The point is that the pixels of a line are all the same, so backgroundPlane only contains the color of each line.
    backgroundPlane[lineNumber * 4]     = (controlRegisters[BackdropColor] & 0x000008) ? 255 : 128;
    backgroundPlane[lineNumber * 4 + 1] = (controlRegisters[BackdropColor] & 0x000004) ? 255 : 0;
    backgroundPlane[lineNumber * 4 + 2] = (controlRegisters[BackdropColor] & 0x000002) ? 255 : 0;
    backgroundPlane[lineNumber * 4 + 3] = (controlRegisters[BackdropColor] & 0x000001) ? 255 : 0;
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

void MCD212::OverlayMix()
{
    uint8_t* dst = screen(lineNumber, 3);
    uint8_t* backgd = backgroundPlane(lineNumber, 4);
    uint8_t* backPlane;
    uint8_t* frontPlane;

    regionFlags[0].fill(false);
    regionFlags[1].fill(false);
    currentRegionControl = RegionControl - 1;

    if(controlRegisters[PlaneOrder] & 1)
    {
        backPlane = planeA(lineNumber, 4);
        frontPlane = planeB(lineNumber, 4);
    }
    else
    {
        backPlane = planeB(lineNumber, 4);
        frontPlane = planeA(lineNumber, 4);
    }

    const int abg = *backgd++;
    const int rbg = *backgd++;
    const int gbg = *backgd++;
    const int bbg = *backgd++;

    for(uint16_t w = 0; w < planeA.width; w++)
    {
        HandleRegions(w);

        uint8_t weightBack;
        uint8_t weightFront;
        if(controlRegisters[PlaneOrder] & 1)
        {
            HandleTransparency(backPlane,  w, controlRegisters[TransparencyControl] & 0xF, controlRegisters[TransparentColorForPlaneA]);
            HandleTransparency(frontPlane, w, controlRegisters[TransparencyControl] >> 8 & 0xF, controlRegisters[TransparentColorForPlaneB]);

            weightBack = controlRegisters[WeightFactorForPlaneA] & 0x0000003F;
            weightFront = controlRegisters[WeightFactorForPlaneB] & 0x0000003F;
        }
        else
        {
            HandleTransparency(backPlane,  w, controlRegisters[TransparencyControl] >> 8 & 0xF, controlRegisters[TransparentColorForPlaneB]);
            HandleTransparency(frontPlane, w, controlRegisters[TransparencyControl] & 0xF, controlRegisters[TransparentColorForPlaneA]);

            weightBack = controlRegisters[WeightFactorForPlaneB] & 0x0000003F;
            weightFront = controlRegisters[WeightFactorForPlaneA] & 0x0000003F;
        }

        int abp = *backPlane++;
        int rbp = *backPlane++;
        int gbp = *backPlane++;
        int bbp = *backPlane++;

        int afp = *frontPlane++;
        int rfp = *frontPlane++;
        int gfp = *frontPlane++;
        int bfp = *frontPlane++;

        rbp = (limu8(rbp - 16) * weightBack) / 63 + 16;
        gbp = (limu8(gbp - 16) * weightBack) / 63 + 16;
        bbp = (limu8(bbp - 16) * weightBack) / 63 + 16;

        rfp = (limu8(rfp - 16) * weightFront) / 63 + 16;
        gfp = (limu8(gfp - 16) * weightFront) / 63 + 16;
        bfp = (limu8(bfp - 16) * weightFront) / 63 + 16;

        int r = 16;
        int g = 16;
        int b = 16;

        if(!(controlRegisters[TransparencyControl] & 0x800000)) // Mixing
        {
            r = limu8(rbp + rfp - 16);
            g = limu8(gbp + gfp - 16);
            b = limu8(bbp + bfp - 16);
        }
        else
        {
            int ap = afp + abp * (255 - afp);
            int rp = 16;
            int gp = 16;
            int bp = 16;

            if(ap > 0)
            {
                rp = (rfp * afp + rbp * abp * (255 - afp)) / ap;
                gp = (gfp * afp + gbp * abp * (255 - afp)) / ap;
                bp = (bfp * afp + bbp * abp * (255 - afp)) / ap;
            }

            if(ap >= 256)
                ap /= 255;
            r = (rp * ap + rbg * abg * (255 - ap)) / 255;
            g = (gp * ap + gbg * abg * (255 - ap)) / 255;
            b = (bp * ap + bbg * abg * (255 - ap)) / 255;
        }

        *dst++ = r;
        *dst++ = g;
        *dst++ = b;
    }
}

void MCD212::HandleTransparency(uint8_t* pixel, const uint16_t pos, const uint32_t control, const uint32_t color)
{
    const uint8_t r = color >> 16;
    const uint8_t g = color >> 8;
    const uint8_t b = color;

    switch(control)
    {
    case 0: // Always
        pixel[0] = 0;
        break;

    case 1: // Color Key = True
        if(pixel[1] == r && pixel[2] == g && pixel[3] == b)
            pixel[0] = 0;
        break;

    case 2: // Transparency Bit = 1
        if(pixel[0] == 255)
            pixel[0] = 0;
        break;

    case 3: // Region Flag 0 = True
        if(regionFlags[0][pos])
            pixel[0] = 0;
        break;

    case 4: // Region Flag 1 = True
        if(regionFlags[1][pos])
            pixel[0] = 0;
        break;

    case 5: // Region Flag 0 or Color Key = True
        if(regionFlags[0][pos] || (pixel[1] == r && pixel[2] == g && pixel[3] == b))
            pixel[0] = 0;
        break;

    case 6: // Region Flag 1 or Color Key = True
        if(regionFlags[1][pos] || (pixel[1] == r && pixel[2] == g && pixel[3] == b))
            pixel[0] = 0;
        break;

    case 8: // Never
        pixel[0] = 255;
        break;

    case 9: // Color Key = False
        if(!(pixel[1] == r && pixel[2] == g && pixel[3] == b))
            pixel[0] = 0;
        break;

    case 10: // Transparency Bit = 0
        if(pixel[0] == 128)
            pixel[0] = 0;
        break;

    case 11: // Region Flag 0 = False
        if(!regionFlags[0][pos])
            pixel[0] = 0;
        break;

    case 12: // Region Flag 1 = False
        if(!regionFlags[1][pos])
            pixel[0] = 0;
        break;

    case 13: // Region Flag 0 or Color Key = False
        if(!regionFlags[0][pos] || !(pixel[1] == r && pixel[2] == g && pixel[3] == b))
            pixel[0] = 0;
        break;

    case 14: // Region Flag 1 or Color Key = False
        if(!regionFlags[1][pos] || !(pixel[1] == r && pixel[2] == g && pixel[3] == b))
            pixel[0] = 0;
        break;
    }
}

void MCD212::HandleRegions(const uint16_t pos)
{
    if(currentRegionControl >= RegionControl + 7)
        return;

    const uint16_t x = controlRegisters[currentRegionControl + 1] >> 1 & 0x1FF; // double resolution
    if(pos >= x)
    {
        currentRegionControl++;
    }

    if(currentRegionControl < RegionControl)
        return;

    const uint8_t op = controlRegisters[currentRegionControl] >> 20 & 0x00000F;
    if(op == 0)
    {
        currentRegionControl = RegionControl + 8;
        return;
    }

    const uint32_t wf = controlRegisters[currentRegionControl] >> 10 & 0x00003F;
    if((op & 0b0110) == 0b0100)
        controlRegisters[WeightFactorForPlaneA] = wf;
    else if((op & 0b0110) == 0b0110)
        controlRegisters[WeightFactorForPlaneB] = wf;

    const bool rf = controlRegisters[ImageCodingMethod] & 0x080000 ? currentRegionControl / 4 : controlRegisters[currentRegionControl] & 0x010000;
    if((op & 0b1001) == 0b1000)
        regionFlags[rf][pos] = false;
    else if((op & 0b1001) == 0b1001)
        regionFlags[rf][pos] = true;
}

void MCD212::DecodeMosaicLineA() // TODO
{
    const Video::ImageCodingMethod codingMethod = ICM_LUT_A[controlRegisters[ImageCodingMethod] & 0x00000F];
    uint8_t* data = &memory[GetVSR1()];
    uint8_t* pixels = &planeA[lineNumber * planeA.width * 4];
    uint16_t index = 0;
    uint32_t previous = controlRegisters[DYUVAbsStartValueForPlaneA];

    for(uint16_t x = 0; x < planeA.width;)
    {
        if(codingMethod == ICM(DYUV))
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
        else if(codingMethod == ICM(CLUT4))
        {
            Video::decodeCLUT((data[index] >> 4 & 0x0F), &pixels[x * 4], CLUT.data());
            Video::decodeCLUT(data[index++] & 0x0F, &pixels[(x+1) * 4], CLUT.data());
            for(uint16_t i = 1; i < GetMF12_1(); i++)
                memcpy(&pixels[(i * 2 + x) * 4], &pixels[x * 4], 8);
            x += GetMF12_1() * 2;
        }
        else // CLUT
        {
            Video::decodeCLUT(data[index++] + (codingMethod == ICM(CLUT77) && controlRegisters[ImageCodingMethod] & 0x400000) ? 128 : 0, &pixels[x * 4], CLUT.data());
            for(uint16_t i = 1; i < GetMF12_1(); i++)
                memcpy(&pixels[(i + x) * 4], &pixels[x * 4], 4);
            x += GetMF12_1();
        }
    }
}

void MCD212::DecodeMosaicLineB() // TODO
{
    const Video::ImageCodingMethod codingMethod = ICM_LUT_B[controlRegisters[ImageCodingMethod] >> 8 & 0x00000F];
    uint8_t* dataA = &memory[GetVSR1()];
    uint8_t* dataB = &memory[GetVSR2()];
    uint8_t* pixels = &planeB[lineNumber * planeB.width * 4];
    uint16_t index = 0;
    uint32_t previous = controlRegisters[DYUVAbsStartValueForPlaneB];

    for(uint16_t x = 0; x < planeB.width;)
    {
        if(codingMethod == ICM(DYUV))
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
        else if(codingMethod == ICM(RGB555))
        {
            uint16_t color  = dataA[index] << 8;
            color |= dataB[index];
            Video::decodeRGB555(color, &pixels[x++ * 4]);
            for(uint16_t i = 1; i < GetMF12_2(); i++)
                memcpy(&pixels[(i + x) * 4], &pixels[x * 4], 4);
            x += GetMF12_2();
        }
        else if(codingMethod == ICM(CLUT4))
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
