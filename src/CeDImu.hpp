#ifndef CEDIMU_HPP
#define CEDIMU_HPP

#include "CDI/CDI.hpp"

#include <wx/app.h>

class CeDImu : public wxApp
{
public:
    CDI m_cdi;
    uint16_t m_cpuSpeed;

    virtual bool OnInit() override;
    virtual int OnExit() override;

    bool InitCDI();
};

#endif // CEDIMU_HPP
