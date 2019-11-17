#ifndef CEDIMU_HPP
#define CEDIMU_HPP

class CeDImu;

#include <thread>

#include <wx/app.h>

#include "CDI/CDI.hpp"
#include "Cores/VDSC.hpp"
#include "GUI/MainFrame.hpp"
#include "Cores/SCC68070/SCC68070.hpp"

class CeDImu : public wxApp
{
public:
    CDI* cdi;
    VDSC* vdsc;
    SCC68070* cpu;

    MainFrame* mainFrame;

    virtual bool OnInit() override;
    virtual int OnExit() override;
    bool InitializeCores(const char* pathToBIOS);
    bool InitializeCDI(const char* pathToROM);
    void StartGameThread();
    void StopGameThread();

private:
    std::thread* gameThread;
};

#endif // CEDIMU_HPP
