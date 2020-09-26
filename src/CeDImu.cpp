#include "CeDImu.hpp"

#include <wx/msgdlg.h>

#include "Config.hpp"

bool CeDImu::OnInit()
{
    cdi = new CDI();

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
    Config::saveConfig();
    return 0;
}

bool CeDImu::InitializeCores(const char* vdscBios, const char* slaveBios)
{
    FILE* f = fopen(vdscBios, "rb");
    if(!f)
    {
        wxMessageBox("Could not open VDSC BIOS file!");
        return false;
    }

    fseek(f, 0, SEEK_END);
    long biosSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t* bios = new uint8_t[biosSize];
    fread(bios, 1, biosSize, f);;
    fclose(f);

    FILE* ff = fopen(slaveBios, "rb");
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

    cdi->LoadBoard(bios, biosSize, slave, slaveSize);
    delete[] bios;
    delete[] slave;

    std::string str(vdscBios);
#ifdef _WIN32
    biosName = str.substr(str.rfind('\\')+1);
#else
    biosName = str.substr(str.rfind('/')+1);
#endif // _WIN32

    cdi->board->SetOnFrameCompletedCallback([=] () -> void {
        mainFrame->gamePanel->RefreshScreen();
    });

    cdi->board->cpu->SetFrequency(cpuFrequencies[cpuFrequencyIndex]);
    return true;
}

bool CeDImu::InitializeCDI(const char* pathToROM)
{
    return cdi->disk.Open(pathToROM);
}

void CeDImu::IncreaseEmulationSpeed()
{
    if(cpuFrequencyIndex < 11)
    {
        cpuFrequencyIndex++;
        if(cdi->board)
            cdi->board->cpu->SetFrequency(cpuFrequencies[cpuFrequencyIndex]);
    }
}

void CeDImu::DecreaseEmulationSpeed()
{
    if(cpuFrequencyIndex > 0)
    {
        cpuFrequencyIndex--;
        if(cdi->board)
            cdi->board->cpu->SetFrequency(cpuFrequencies[cpuFrequencyIndex]);
    }
}

void CeDImu::StartGameThread()
{
    if(cdi->board)
        cdi->board->cpu->Run(true);
}

void CeDImu::StopGameThread()
{
    if(cdi->board)
        cdi->board->cpu->Stop();
}
