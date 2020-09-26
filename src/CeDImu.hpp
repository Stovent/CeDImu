#ifndef CEDIMU_HPP
#define CEDIMU_HPP

class CeDImu;

#include <thread>

#include <wx/app.h>

#include "CDI/CDI.hpp"
#include "cores/VDSC.hpp"
#include "GUI/MainFrame.hpp"
#include "cores/SCC68070/SCC68070.hpp"

class CeDImu : public wxApp
{
public:
    CDI* cdi;
    std::string biosName;

    MainFrame* mainFrame;
    uint32_t cpuFrequencies[12] = {
        uint32_t(SCC68070_DEFAULT_FREQUENCY * 0.12),
        uint32_t(SCC68070_DEFAULT_FREQUENCY * 0.25),
        uint32_t(SCC68070_DEFAULT_FREQUENCY * 0.33),
        uint32_t(SCC68070_DEFAULT_FREQUENCY * 0.5),
        uint32_t(SCC68070_DEFAULT_FREQUENCY * 0.75),
        uint32_t(SCC68070_DEFAULT_FREQUENCY),
        uint32_t(SCC68070_DEFAULT_FREQUENCY * 1.5),
        uint32_t(SCC68070_DEFAULT_FREQUENCY * 2),
        uint32_t(SCC68070_DEFAULT_FREQUENCY * 3),
        uint32_t(SCC68070_DEFAULT_FREQUENCY * 4),
        uint32_t(SCC68070_DEFAULT_FREQUENCY * 8),
        uint32_t(SCC68070_DEFAULT_FREQUENCY * 16),
    };
    uint16_t cpuFrequencyIndex;

    virtual bool OnInit() override;
    virtual int OnExit() override;
    bool InitializeCores(const char* vdscBios, const char* slaveBios);
    bool InitializeCDI(const char* pathToROM);
    void IncreaseEmulationSpeed();
    void DecreaseEmulationSpeed();
    void StartGameThread();
    void StopGameThread();
};

#endif // CEDIMU_HPP
