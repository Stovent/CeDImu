#include "CeDImu.hpp"

#include "Cores/SCC66470/SCC66470.hpp"
#include "Cores/MCD212/MCD212.hpp"

bool CeDImu::OnInit()
{
    cdi = nullptr;
    vdsc = nullptr;
    cpu = nullptr;
    gameThread = nullptr;

    mainFrame = new MainFrame(this, "CeDImu", wxPoint(50, 50), wxSize(384, 240));
    mainFrame->Show(true);
    return true;
}

void CeDImu::InitializeCores(const char* pathToBIOS)
{
    FILE* f = fopen(pathToBIOS, "rb");
    fseek(f, 0, SEEK_END);
    long biosSize = ftell(f);
    fclose(f);

    delete vdsc;
    delete cpu;

    if(biosSize == 523264)
        vdsc = new SCC66470(this);
    else
        vdsc = new MCD212(this);

    cpu = new SCC68070(this, vdsc);
    vdsc->LoadBIOS(pathToBIOS);
}

bool CeDImu::InitializeCDI(const char* pathToROM)
{
    delete cdi;
    cdi = new CDI(this);
    return cdi->OpenROM(pathToROM);
}

void CeDImu::StartGameThread()
{
    if(gameThread == nullptr && cpu != nullptr)
    {
        cpu->run = true;
        gameThread = new std::thread(&SCC68070::Run, cpu);
    }
}

void CeDImu::StopGameThread()
{
    if(gameThread == nullptr)
        return;
    cpu->run = false;
    if(gameThread->joinable())
        gameThread->join();
    delete gameThread;
    gameThread = nullptr;
}
