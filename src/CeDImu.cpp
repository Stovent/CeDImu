#include "CeDImu.hpp"
#include "GUI/MainFrame.hpp"

bool CeDImu::OnInit()
{
    MainFrame* mainFrame = new MainFrame(this, "CeDImu", wxPoint(50, 50), wxSize(384, 240));
    mainFrame->Show(true);
    return true;
}

