#ifndef CEDIMU_HPP
#define CEDIMU_HPP

class CeDImu;

#include "CDI/CDI.hpp"
#include "CDI/cores/VDSC.hpp"
#include "CDI/cores/SCC68070/SCC68070.hpp"
#include "GUI/MainFrame.hpp"

#include <wx/app.h>

#include <atomic>
#include <cstdio>

class CeDImu : public wxApp
{
public:
    std::atomic<bool> stopOnNextFrame;
    CDI cdi;

    std::string biosName;
    uint16_t cpuSpeed;

    std::ofstream uartOut;
    std::ofstream logInstructions;
    std::ofstream logMemoryAccess;

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
