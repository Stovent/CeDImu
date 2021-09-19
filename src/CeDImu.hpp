#ifndef CEDIMU_HPP
#define CEDIMU_HPP

#include "CDI/CDI.hpp"

#include <wx/app.h>

#include <mutex>

extern const float CPU_SPEEDS[];

class CeDImu : public wxApp
{
public:
    std::mutex m_cdiMutex;
    CDI m_cdi;
    uint16_t m_cpuSpeed;

    virtual bool OnInit() override;
    virtual int OnExit() override;

    bool InitCDI();
    void StartEmulation();
    void StopEmulation();
    void IncreaseEmulationSpeed();
    void DecreaseEmulationSpeed();
};

#endif // CEDIMU_HPP
