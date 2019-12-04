#ifndef GAMEPANEL_HPP
#define GAMEPANEL_HPP

class GamePanel;

#include <wx/panel.h>
#include <wx/timer.h>
#include <wx/dcclient.h>

#include "../CeDImu.hpp"
#include "MainFrame.hpp"

class GamePanel : public wxPanel
{
public:
    GamePanel(MainFrame* parent, CeDImu* appp);
    ~GamePanel();

    void RefreshScreen(const wxImage& img);

private:
    CeDImu* app;
    MainFrame* mainFrame;
    wxTimer* renderTimer;
    wxImage screen;
    unsigned long long oldInstCount;

    void OnKeyDown(wxKeyEvent& event);
    void RefreshLoop(wxTimerEvent& event);
    void DrawTextInfo(wxClientDC& dc);

    wxDECLARE_EVENT_TABLE();
};

#endif // GAMEPANEL_HPP
