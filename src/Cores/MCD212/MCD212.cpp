#include "MCD212.hpp"

#include <cstring>
#include <wx/msgdlg.h>

#define   SET_DA_BIT() internalRegisters[CSR1R] |= 0x80;
#define UNSET_DA_BIT() internalRegisters[CSR1R] &= 0x20;

MCD212::MCD212(CeDImu* appp) : VDSC(appp) // TD = 0
{
    controlRegisters = new uint32_t[0x80];
    internalRegisters = new uint16_t[32];
    allocatedMemory = 0x500000;
    memory = new uint8_t[allocatedMemory];
    memorySwapCount = 0;
    isCA = false;

    Reset();

#ifdef DEBUG
    out.open("MCD212.txt");
#endif // DEBUG

    backgroundPlane.Create(1, 1);
    if(!backgroundPlane.HasAlpha())
        backgroundPlane.InitAlpha();
    cursorPlane.Create(16, 16);
    if(!cursorPlane.HasAlpha())
        cursorPlane.InitAlpha();
}

MCD212::~MCD212()
{
    delete[] controlRegisters;
    delete[] internalRegisters;
}

void MCD212::Reset()
{
    memset(controlRegisters, 0, 0x80*sizeof(uint32_t));
    memset(internalRegisters, 0, 32*sizeof(uint16_t));
    ResetMemory();

    controlRegisters[ImageCodingMethod] &= 0xF80000; // reset bits 0, 1, 2, 3, 8, 9, 10, 11, 18 (plane A and B off,external video disabled)
    controlRegisters[CursorControl] &= 0x7FFFFF; // reset bit 23 (cursor disabled)
    controlRegisters[BackdropColor] = 0; // reset bits 0, 1, 2, 3 (black backdrop)

    internalRegisters[CSR1W] = 0; // DI1, DD1, DD2, TD, DD, ST, BE
    internalRegisters[CSR2W] = 0; // DI2
    internalRegisters[DCR1] &= 0x003F; // DE, CF, FD, SM, CM1, IC1, DC1
    internalRegisters[DCR2] &= 0x003F; // CM2, IC2, DC2
    internalRegisters[DDR1] &= 0x003F; // MF1, MF2, FT1, FT2
    internalRegisters[DDR2] &= 0x003F; // MF1, MF2, FT1, FT2
}

bool MCD212::LoadBIOS(const char* filename)
{
    biosLoaded = false;
    FILE* f = fopen(filename, "rb");
    if(f == NULL)
        return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f); // should be 512KB
    fseek(f, 0, SEEK_SET);

    fread(&memory[0x400000], 1, size, f);

    fclose(f);
    std::string str(filename);
#ifdef _WIN32
    biosFilename = str.substr(str.rfind('\\')+1);
#else
    biosFilename = str.substr(str.rfind('/')+1);
#endif // _WIN32
    return biosLoaded = true;
}

void MCD212::PutDataInMemory(const void* s, unsigned int size, unsigned int position)
{
    memcpy(&memory[position], s, size);
}

void MCD212::ResetMemory()
{
    memset(memory, 0, allocatedMemory);
}

void MCD212::MemorySwap()
{
    memorySwapCount = 0;
}

void MCD212::DisplayLine()
{
    SET_DA_BIT()

    if(GetDE())
    {
        DisplayLineA();
        DisplayLineB();
        if(controlRegisters[CursorControl] & 0x800000) // Cursor enable bit
            DrawCursor();
    }

    if(++lineNumber >= GetVerticalResolution())
    {
        lineNumber = 0;
        UNSET_DA_BIT()
        if(GetDE())
        {
            DrawPlaneA();
            DrawPlaneB();
            DrawBackground();

            wxImage* screen = new wxImage(GetHorizontalResolution1(), GetVerticalResolution());
            app->mainFrame->gamePanel->RefreshScreen(backgroundPlane);
            delete screen;
        }
        totalFrameCount++;
        OnFrameCompleted();
    }
}

void MCD212::DisplayLineA()
{
    if(GetIC1() && GetDC1())
        ExecuteDCA1();

    if(GetIC1() && lineNumber >= GetVerticalResolution()-1)
        ExecuteICA1();
}

void MCD212::DisplayLineB()
{
    if(GetIC2() && GetDC2())
        ExecuteDCA2();

    if(GetIC2() && lineNumber >= GetVerticalResolution()-1)
        ExecuteICA2();
}

void MCD212::ExecuteICA1()
{
    isCA = true;
    uint32_t ica;
    uint32_t addr = 0x400;
    for(uint16_t i = 0; i < 2000; i++) // change 2000 with value in section 5.4.1
    {
        ica = GetLong(addr + 4*i);
        switch(ica >> 28)
        {
        case 0: // STOP
            goto end_ICA1;
            break;

        case 1: // NOP
            break;

        case 2: // RELOAD DCP
            SetDCP1(ica & 0x003FFFFC);
            break;

        case 3: // RELOAD DCP AND STOP
            SetDCP1(ica & 0x003FFFFC);
            goto end_ICA1;
            break;

        case 4: // RELOAD ICA
            addr = ica & 0x003FFFFF;
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR1(ica & 0x003FFFFF);
            goto end_ICA1;
            break;

        case 6: // INTERRUPT
            SetIT1();
            break;

        case 7: // RELOAD DISPLAY PARAMETERS
            ReloadDisplayParameters1((ica >> 4) & 1, (ica >> 2) & 3, ica & 3);
            break;

        default:
            controlRegisters[(ica >> 24) - 0x80] = ica & 0x00FFFFFF;
        }
#ifdef DEBUG
        out << std::hex << (addr + 4*i) << "\tICA1 instruction: 0x" << ica << std::endl;
#endif // DEBUG
    }
end_ICA1:
    isCA = false;
    return;
}

void MCD212::ExecuteDCA1()
{
    isCA = true;
    uint32_t dca;
    uint32_t addr = GetDCP1() + 64*lineNumber;
    for(uint8_t i = 0; i < 16; i++)
    {
        dca = GetLong(addr);
        switch(dca >> 28)
        {
        case 0: // STOP
            goto end_DCA1;
            break;

        case 1: // NOP
            break;

        case 2: // RELOAD DCP
            SetDCP1(dca & 0x003FFFFC);
            break;

        case 3: // RELOAD DCP AND STOP
            SetDCP1(dca & 0x003FFFFC);
            goto end_DCA1;
            break;

        case 4: // RELOAD VSR
            SetVSR1(dca & 0x003FFFFF);
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR1(dca & 0x003FFFFF);
            goto end_DCA1;
            break;

        case 6: // INTERRUPT
            SetIT1();
            break;

        case 7: // RELOAD DISPLAY PARAMETERS
            ReloadDisplayParameters1((dca >> 4) & 1, (dca >> 2) & 3, dca & 3);
            break;

        default:
            controlRegisters[(dca >> 24) - 0x80] = dca & 0x00FFFFFF;
        }
#ifdef DEBUG
        out << std::hex << addr << "\tDCA1 instruction: 0x" << dca << std::endl;
#endif // DEBUG
        addr += 4;
    }
end_DCA1:
    isCA = false;
    return;
}

void MCD212::ExecuteICA2()
{
    isCA = true;
    uint32_t ica;
    uint32_t addr = 0x200400;
    for(uint16_t i = 0; i < 2000; i++) // change 2000 with value in section 5.4.1
    {
        ica = GetLong(addr + 4*i);
        switch(ica >> 28)
        {
        case 0: // STOP
            goto end_ICA2;
            break;

        case 1: // NOP
            break;

        case 2: // RELOAD DCP
            SetDCP2(ica & 0x003FFFFC);
            break;

        case 3: // RELOAD DCP AND STOP
            SetDCP2(ica & 0x003FFFFC);
            goto end_ICA2;
            break;

        case 4: // RELOAD ICA
            addr = ica & 0x003FFFFF;
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR2(ica & 0x003FFFFF);
            goto end_ICA2;
            break;

        case 6: // INTERRUPT
            SetIT2();
            break;

        case 7: // RELOAD DISPLAY PARAMETERS
            ReloadDisplayParameters2((ica >> 4) & 1, (ica >> 2) & 3, ica & 3);
            break;

        default:
            controlRegisters[(ica >> 24) - 0x80] = ica & 0x00FFFFFF;
        }
#ifdef DEBUG
        out << std::hex << (addr + 4*i) << "\tICA2 instruction: 0x" << ica << std::endl;
#endif // DEBUG
    }
end_ICA2:
    isCA = false;
    return;
}

void MCD212::ExecuteDCA2()
{
    isCA = true;
    uint32_t dca;
    uint32_t addr = GetDCP2() + 64*lineNumber;
    for(uint8_t i = 0; i < 16; i++)
    {
        dca = GetLong(addr);
        switch(dca >> 28)
        {
        case 0: // STOP
            goto end_DCA2;
            break;

        case 1: // NOP
            break;

        case 2: // RELOAD DCP
            SetDCP2(dca & 0x003FFFFC);
            break;

        case 3: // RELOAD DCP AND STOP
            SetDCP2(dca & 0x003FFFFC);
            goto end_DCA2;
            break;

        case 4: // RELOAD VSR
            SetVSR2(dca & 0x003FFFFF);
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR2(dca & 0x003FFFFF);
            goto end_DCA2;
            break;

        case 6: // INTERRUPT
            SetIT2();
            break;

        case 7: // RELOAD DISPLAY PARAMETERS
            ReloadDisplayParameters2((dca >> 4) & 1, (dca >> 2) & 3, dca & 3);
            break;

        default:
            controlRegisters[(dca >> 24) - 0x80] = dca & 0x00FFFFFF;
        }
#ifdef DEBUG
        out << std::hex << addr << "\tDCA2 instruction: 0x" << dca << std::endl;
#endif // DEBUG
        addr += 4;
    }
end_DCA2:
    isCA = false;
    return;
}

void MCD212::DrawPlaneA()
{
    uint16_t width = GetHorizontalResolution1();
    uint16_t height = GetVerticalResolution();
    if(!planeA.Create(width, height))
    {
        wxMessageBox("Could not create plane A image");
        return;
    }

    if(GetFT12_1() <= 1)
    {
        DecodeBitmap(&memory[GetVSR1()], planeA.GetData(), GetHorizontalResolution1(), GetVerticalResolution(), GetCM1());
    }
    else if(GetFT12_1() == 2)
    {
        DecodeRunLength(&memory[GetVSR1()], planeA.GetData(), GetHorizontalResolution1(), GetVerticalResolution(), GetCM1());
    }
    else
    {
        DecodeMosaic(&memory[GetVSR1()], planeA.GetData(), GetHorizontalResolution1(), GetVerticalResolution(), GetCM1());
    }
}

void MCD212::DrawPlaneB()
{
    uint16_t width = GetHorizontalResolution2();
    uint16_t height = GetVerticalResolution();
    if(!planeB.Create(width, height))
    {
        wxMessageBox("Could not create plane B image");
        return;
    }
}

void* MCD212::DecodeBitmap(uint8_t* data, uint8_t* dst, uint16_t width, uint16_t height, bool cm)
{
    return 0;
}

void* MCD212::DecodeRunLength(uint8_t* data, uint8_t* dst, uint16_t width, uint16_t height, bool cm)
{
    uint16_t index = 0;
    for(int y = 0; y < height; y++)
    {
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
    return 0;
}

void* MCD212::DecodeMosaic(uint8_t* data, uint8_t* dst, uint16_t width, uint16_t height, bool cm)
{
    return 0;
}

uint32_t MCD212::GetColorFromCLUT(uint8_t address)
{
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
    uint8_t* data = cursorPlane.GetData() + 3*yAddress*15;
    uint8_t* alpha = cursorPlane.GetAlpha();

    uint16_t mask = 1 << 15;
    while(mask)
    {
        if(controlRegisters[CursorPattern] & mask)
        {
            *alpha  = (controlRegisters[BackdropColor] & 0x000008) ? 255 : 128;
            data[0] = (controlRegisters[BackdropColor] & 0x000004) ? 255 : 0;
            data[1] = (controlRegisters[BackdropColor] & 0x000002) ? 255 : 0;
            data[2] = (controlRegisters[BackdropColor] & 0x000001) ? 255 : 0;
        }
        mask >>= 1;
    }
}

void MCD212::OnFrameCompleted()
{
    if(stopOnNextCompletedFrame)
    {
        stopOnNextCompletedFrame = false;
        app->cpu->run = false;
        app->mainFrame->pause->Check(true);
        app->mainFrame->SetStatusText("pause");
    }
}

void MCD212::ShowViewer()
{
    wxFrame* frame = new wxFrame(app->mainFrame, wxID_ANY, "MCD212 Viewer");

    frame->Show();
}
