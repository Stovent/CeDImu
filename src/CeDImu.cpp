#include "CeDImu.hpp"
#include "Config.hpp"

#include <wx/msgdlg.h>

bool CeDImu::OnInit()
{
    Config::loadConfig();

    cpuFrequencyIndex = 5;

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

    FILE* ff = fopen(Config::slaveBIOS.c_str(), "rb");
    if(!ff)
    {
        wxMessageBox("Could not open slave BIOS file!");
        return false;
    }

    fseek(ff, 0, SEEK_END);
    long slaveSize = ftell(ff);
    fseek(ff, 0, SEEK_SET);
    uint8_t* slave = new uint8_t[slaveSize];
    fread(slave, 1, slaveSize, ff);;
    fclose(ff);

    cdi.LoadBoard(bios, biosSize, slave, slaveSize);
    delete[] bios;
    delete[] slave;

#ifdef _WIN32
    biosName = Config::systemBIOS.substr(Config::systemBIOS.rfind('\\')+1);
#else
    biosName = Config::systemBIOS.substr(Config::systemBIOS.rfind('/')+1);
#endif // _WIN32

    cdi.board->SetOnFrameCompletedCallback([=] () -> void {
        mainFrame->gamePanel->RefreshScreen();
    });

    cdi.board->cpu.SetFrequency(cpuFrequencies[cpuFrequencyIndex]);
    return true;
}

void CeDImu::IncreaseEmulationSpeed()
{
    if(cpuFrequencyIndex < 11)
    {
        cpuFrequencyIndex++;
        if(cdi.board)
            cdi.board->cpu.SetFrequency(cpuFrequencies[cpuFrequencyIndex]);
    }
}

void CeDImu::DecreaseEmulationSpeed()
{
    if(cpuFrequencyIndex > 0)
    {
        cpuFrequencyIndex--;
        if(cdi.board)
            cdi.board->cpu.SetFrequency(cpuFrequencies[cpuFrequencyIndex]);
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
