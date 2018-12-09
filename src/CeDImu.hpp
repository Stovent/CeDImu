#ifndef CEDIMU_HPP
#define CEDIMU_HPP

class CeDImu;

#include <thread>

#include <wx/app.h>

#include "Cores/VDSC.hpp"
#include "GUI/MainFrame.hpp"
#include "Cores/SCC68070/SCC68070.hpp"

class CeDImu : public wxApp
{
public:
    VDSC* vdsc;
    SCC68070* cpu;

    MainFrame* mainFrame;

    virtual bool OnInit();
    void StartGameThread();
    void StopGameThread();

private:
    std::thread* gameThread;
};

void launchGameThread(CeDImu* app);

#endif // CEDIMU_HPP
