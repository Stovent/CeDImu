#ifndef CPUVIEWER_HPP
#define CPUVIEWER_HPP

class CPUViewer;

#include "MainFrame.hpp"
#include "../CDI/cores/SCC68070/SCC68070.hpp"

#include <wx/frame.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/timer.h>
#include <wx/aui/framemanager.h>

#include <sstream>

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

    wxListCtrl* internalRegisters;
    wxTextCtrl* uartOut;
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
