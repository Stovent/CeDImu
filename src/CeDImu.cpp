#include "CeDImu.hpp"
#include "Config.hpp"
#include "GUI/MainFrame.hpp"

#include <wx/msgdlg.h>

constexpr float cpuSpeeds[17] = {
    0.01,
    0.03,
    0.06,
    0.12,
    0.25,
    0.33,
    0.5,
    0.75,
    1.0,
    1.5,
    2,
    3,
    4,
    8,
    16,
    32,
    64,
};

bool CeDImu::OnInit()
{
    Config::loadConfig();
    m_cpuSpeed = 8;

    new MainFrame();

    return true;
}

int CeDImu::OnExit()
{
    Config::saveConfig();
    return 0;
}
