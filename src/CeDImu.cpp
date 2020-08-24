#include "CeDImu.hpp"

#include <wx/msgdlg.h>

#include "Config.hpp"

bool CeDImu::OnInit()
{
    cdi = nullptr;
    vdsc = nullptr;
    cpu = nullptr;

    Config::loadConfig();

    cpuFrequencyIndex = 5;

    mainFrame = new MainFrame(this, "CeDImu", wxPoint(50, 50), wxSize(384, 240));
    mainFrame->Show(true);

    return true;
}

int CeDImu::OnExit()
{
    StopGameThread();
    delete cdi;
    delete vdsc;
    delete cpu;
    Config::saveConfig();
    return 0;
}

bool CeDImu::InitializeCores(const char* pathToBIOS)
{
    FILE* f = fopen(pathToBIOS, "rb");
    if(!f)
    {
        wxMessageBox("Could not open BIOS file!");
        return false;
    }
    fseek(f, 0, SEEK_END);
    long biosSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t* d = new uint8_t[biosSize];
    fread(d, 1, biosSize, f);;
    fclose(f);

    delete vdsc;
    delete cpu;

    if(biosSize == 523264)
    {
        vdsc = new SCC66470(this);
        wxMessageBox("WARNING: this BIOS is not implemented yet, it may not work correctly. Please use another one");
    }
    else
        vdsc = new MCD212(this);

    if(!vdsc->LoadBIOS(d, biosSize))
        return false;

    std::string str(pathToBIOS);
#ifdef _WIN32
    biosName = str.substr(str.rfind('\\')+1);
#else
    biosName = str.substr(str.rfind('/')+1);
#endif // _WIN32
    vdsc->SetOnFrameCompletedCallback([=] () -> void {
        mainFrame->gamePanel->RefreshScreen();
    });

    cpu = new SCC68070(vdsc);
    cpu->SetFrequency(cpuFrequencies[cpuFrequencyIndex]);
    return true;
}

bool CeDImu::InitializeCDI(const char* pathToROM)
{
    delete cdi;
    cdi = new CDI();
    return cdi->OpenROM(pathToROM);
}

void CeDImu::IncreaseEmulationSpeed()
{
    if(cpuFrequencyIndex < 11)
    {
        cpuFrequencyIndex++;
        if(cpu)
            cpu->SetFrequency(cpuFrequencies[cpuFrequencyIndex]);
    }
}

void CeDImu::DecreaseEmulationSpeed()
{
    if(cpuFrequencyIndex > 0)
    {
        cpuFrequencyIndex--;
        if(cpu)
            cpu->SetFrequency(cpuFrequencies[cpuFrequencyIndex]);
    }
}

void CeDImu::StartGameThread()
{
    if(cpu)
        cpu->Run(true);
}

void CeDImu::StopGameThread()
{
    if(cpu)
        cpu->Stop();
}
