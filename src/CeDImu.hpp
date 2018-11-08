#ifndef CEDIMU_HPP
#define CEDIMU_HPP

class CeDImu;

#include <thread>

#include <wx/wx.h>

class CeDImu : public wxApp
{
public:
    virtual bool OnInit();

private:
    std::thread gameThread;
};

#endif // CEDIMU_HPP
