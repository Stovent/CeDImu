#ifndef CPUVIEWER_HPP
#define CPUVIEWER_HPP

class CPUViewer;

#include <sstream>

#include <wx/textctrl.h>
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/timer.h>
#include <wx/aui/framemanager.h>

#include "../cores/SCC68070/SCC68070.hpp"
#include "MainFrame.hpp"

class CPUViewer : public wxFrame
{
public:
    std::stringstream instructions;

    CPUViewer(SCC68070* core, MainFrame* parent, const wxPoint& pos, const wxSize& size);
    ~CPUViewer();

    wxAuiManager auiManager;

    SCC68070* cpu;
    MainFrame* mainFrame;
    wxTimer* renderTimer;

    wxTextCtrl* disassembler;
    wxTextCtrl* d[8]; wxTextCtrl* a[8];

    wxTextCtrl* pc; wxTextCtrl* sr;

    void OnClose(wxCloseEvent& event);
    void PaintEvent(wxPaintEvent& event);
    void PaintEvent();
    void RefreshLoop(wxTimerEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // CPUVIEWER_HPP
