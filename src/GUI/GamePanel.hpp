#ifndef GAMEPANEL_HPP
#define GAMEPANEL_HPP

class GamePanel;

#include <wx/panel.h>
#include <wx/dcclient.h>

#include "../CeDImu.hpp"
#include "MainFrame.hpp"

class GamePanel : public wxPanel
{
public:
    CeDImu* app;
    MainFrame* mainFrame;

    GamePanel(MainFrame* parent, CeDImu* appp);
    ~GamePanel();

    void RefreshScreen();

    void OnKeyDown(wxKeyEvent& event);
    void RefreshLoop(wxPaintEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // GAMEPANEL_HPP
