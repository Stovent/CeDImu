#ifndef GAMEPANEL_HPP
#define GAMEPANEL_HPP

class GamePanel;

#include <wx/wx.h>

#include "../CeDImu.hpp"
#include "MainFrame.hpp"

class GamePanel : public wxPanel
{
public:
    GamePanel(MainFrame* parent, CeDImu* appp);

private:
    CeDImu* app;
    MainFrame* mainFrame;
};

#endif // GAMEPANEL_HPP
