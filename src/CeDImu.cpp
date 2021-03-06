#include "CeDImu.hpp"
#include "Config.hpp"

#include <wx/msgdlg.h>

constexpr float cpuSpeeds[12] = {
    0.12,
    0.25,
    0.33,
    0.5,
    0.75,
    1.0,
    1.5,
    2,
    3,
    4,
    8,
    16,
};

bool CeDImu::OnInit()
{
    Config::loadConfig();

    cpuSpeed = 5;

    mainFrame = new MainFrame(this, "CeDImu", wxPoint(50, 50), wxSize(384, 240));
    mainFrame->Show(true);

    InitializeCores();
    return true;
}

int CeDImu::OnExit()
{
    StopGameThread();
    Config::saveConfig();
    return 0;
}

bool CeDImu::InitializeCores()
{
    FILE* f = fopen(Config::systemBIOS.c_str(), "rb");
    if(!f)
    {
        wxMessageBox("Could not open system BIOS file!");
        return false;
    }

    fseek(f, 0, SEEK_END);
    long biosSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t* bios = new uint8_t[biosSize];
    fread(bios, 1, biosSize, f);;
    fclose(f);

    cdi.LoadBoard(bios, biosSize, Config::NVRAMUseCurrentTime);
    delete[] bios;

#ifdef _WIN32
    biosName = Config::systemBIOS.substr(Config::systemBIOS.rfind('\\')+1);
#else
    biosName = Config::systemBIOS.substr(Config::systemBIOS.rfind('/')+1);
#endif // _WIN32

    cdi.board->SetOnFrameCompletedCallback([=] () -> void {
        mainFrame->gamePanel->RefreshScreen();
    });

    cdi.board->cpu.SetEmulationSpeed(cpuSpeeds[cpuSpeed]);
    return true;
}

void CeDImu::IncreaseEmulationSpeed()
{
    if(cpuSpeed < 11)
    {
        cpuSpeed++;
        if(cdi.board)
            cdi.board->cpu.SetEmulationSpeed(cpuSpeeds[cpuSpeed]);
    }
}

void CeDImu::DecreaseEmulationSpeed()
{
    if(cpuSpeed > 0)
    {
        cpuSpeed--;
        if(cdi.board)
            cdi.board->cpu.SetEmulationSpeed(cpuSpeeds[cpuSpeed]);
    }
}

void CeDImu::StartGameThread()
{
    if(cdi.board)
        cdi.board->cpu.Run(true);
}

void CeDImu::StopGameThread()
{
    if(cdi.board)
        cdi.board->cpu.Stop();
}
