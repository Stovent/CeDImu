#ifndef CEDIMU_HPP
#define CEDIMU_HPP

class CeDImu;

#include "CDI/CDI.hpp"
#include "CDI/cores/VDSC.hpp"
#include "CDI/cores/SCC68070/SCC68070.hpp"
#include "GUI/MainFrame.hpp"

#include <wx/app.h>

#include <thread>

class CeDImu : public wxApp
{
public:
    CDI cdi;
    std::string biosName;
    std::ofstream uart_out;
    uint16_t cpuSpeed;

    MainFrame* mainFrame;

    virtual bool OnInit() override;
    virtual int OnExit() override;
    bool InitializeCores();
    void IncreaseEmulationSpeed();
    void DecreaseEmulationSpeed();
    void StartGameThread();
    void StopGameThread();
};

#endif // CEDIMU_HPP
