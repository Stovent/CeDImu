#ifndef GAMEPANEL_HPP
#define GAMEPANEL_HPP

class GamePanel;

#include <wx/panel.h>

#include "../CeDImu.hpp"
#include "MainFrame.hpp"

class GamePanel : public wxPanel
{
public:
    GamePanel(MainFrame* parent, CeDImu* appp);

private:
    CeDImu* app;
    MainFrame* mainFrame;

    void OnKeyDown(wxKeyEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // GAMEPANEL_HPP
