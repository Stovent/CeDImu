#include "CeDImu.hpp"
#include "GUI/MainFrame.hpp"

void launchGameThread(CeDImu* app)
{
    app->cpu->Run();
}

bool CeDImu::OnInit()
{
//    gameThread = nullptr;
    cpu = new SCC68070();

    MainFrame* mainFrame = new MainFrame(this, "CeDImu", wxPoint(50, 50), wxSize(384, 240));
    mainFrame->Show(true);
    return true;
}

void CeDImu::StartGameThread()
{
    cpu->run = true;
    if(gameThread == nullptr)
        gameThread = new std::thread(launchGameThread, this);
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
