#ifndef CEDIMU_HPP
#define CEDIMU_HPP

class CeDImu;

#include <thread>

#include <wx/app.h>

#include "Cores/SCC68070/SCC68070.hpp"

class CeDImu : public wxApp
{
public:
    SCC68070* cpu;

    virtual bool OnInit();
    void StartGameThread();
    void StopGameThread();

private:
    std::thread* gameThread;
};

void launchGameThread(CeDImu* app);

#endif // CEDIMU_HPP
