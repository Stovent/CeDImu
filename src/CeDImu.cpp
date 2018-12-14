#include "CeDImu.hpp"

void launchGameThread(CeDImu* app)
{
    app->cpu->Run();
}

bool CeDImu::OnInit()
{
    vdsc = new SCC66470();
    cpu = new SCC68070(*this, *vdsc);

    mainFrame = new MainFrame(this, "CeDImu", wxPoint(50, 50), wxSize(384, 240));
    mainFrame->Show(true);
    return true;
}

void CeDImu::StartGameThread()
{
    if(gameThread == nullptr)
    {
        cpu->run = true;
        gameThread = new std::thread(launchGameThread, this);
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
