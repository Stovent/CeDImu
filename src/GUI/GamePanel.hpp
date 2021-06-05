#ifndef GAMEPANEL_HPP
#define GAMEPANEL_HPP

class GamePanel;

#include "../CeDImu.hpp"
#include "MainFrame.hpp"

#include <wx/panel.h>
#include <wx/dcclient.h>

class GamePanel : public wxPanel
{
public:
    CeDImu& app;
    MainFrame* mainFrame;

    int frameWidth;
    int frameHeight;

    GamePanel(MainFrame* parent, CeDImu& appp);
    ~GamePanel();

    void RefreshScreen();

    void OnKeyDown(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);
    void RefreshLoop(wxPaintEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // GAMEPANEL_HPP
