#include "CeDImu.hpp"

#include "Cores/SCC66470/SCC66470.hpp"
#include "Cores/MCD212/MCD212.hpp"

bool CeDImu::OnInit()
{
    cdi = new CDI(this);
//    vdsc = new SCC66470();
    vdsc = new MCD212(this);
    cpu = new SCC68070(*this, *vdsc);
    gameThread = nullptr;

    mainFrame = new MainFrame(this, "CeDImu", wxPoint(50, 50), wxSize(384, 240));
    mainFrame->Show(true);
    return true;
}

void CeDImu::StartGameThread()
{
    if(gameThread == nullptr)
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
