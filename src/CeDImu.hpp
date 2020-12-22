#ifndef CEDIMU_HPP
#define CEDIMU_HPP

class CeDImu;

#include "CDI/CDI.hpp"
#include "cores/VDSC.hpp"
#include "GUI/MainFrame.hpp"
#include "cores/SCC68070/SCC68070.hpp"

#include <wx/app.h>

#include <thread>

class CeDImu : public wxApp
{
public:
    CDI cdi;
    std::string biosName;

    MainFrame* mainFrame;
    uint16_t cpuSpeed;

    virtual bool OnInit() override;
    virtual int OnExit() override;
    bool InitializeCores();
    void IncreaseEmulationSpeed();
    void DecreaseEmulationSpeed();
    void StartGameThread();
    void StopGameThread();
};

#endif // CEDIMU_HPP
