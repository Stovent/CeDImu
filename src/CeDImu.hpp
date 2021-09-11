#ifndef CEDIMU_HPP
#define CEDIMU_HPP

#include "CDI/CDI.hpp"

#include <wx/app.h>

#include <mutex>

class CeDImu : public wxApp
{
public:
    std::mutex m_cdiMutex;
    CDI m_cdi;
    uint16_t m_cpuSpeed;

    virtual bool OnInit() override;
    virtual int OnExit() override;

    bool InitCDI();
};

#endif // CEDIMU_HPP
