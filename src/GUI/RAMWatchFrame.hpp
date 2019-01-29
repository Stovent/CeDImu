#ifndef RAMWATCHFRAME_HPP
#define RAMWATCHFRAME_HPP

class RAMWatchFrame;

#include <wx/frame.h>
#include <wx/timer.h>
#include <wx/grid.h>

#include "MainFrame.hpp"
#include "../Cores/VDSC.hpp"

class RAMWatchFrame : public wxFrame
{
public:
    VDSC* vdsc;
    MainFrame* mainFrame;
    wxGrid* grid;
    wxTimer* renderTimer;

    RAMWatchFrame(VDSC* vds, MainFrame* parent, const wxPoint& pos, const wxSize& size);
    ~RAMWatchFrame();

    void OnClose(wxCloseEvent& event);

    void PaintEvent();
    void RefreshLoop(wxTimerEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // RAMWATCHFRAME_HPP
