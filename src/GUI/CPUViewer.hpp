#ifndef CPUVIEWER_HPP
#define CPUVIEWER_HPP

class CPUViewer;

#include "MainFrame.hpp"
#include "GenericList.hpp"
#include "../CDI/cores/SCC68070/SCC68070.hpp"

#include <wx/frame.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/timer.h>
#include <wx/aui/framemanager.h>

#include <vector>

class CPUViewer : public wxFrame
{
public:
    bool flushInstructions;
    std::vector<Instruction> instructions;

    CPUViewer(CDI& idc, MainFrame* parent, const wxPoint& pos, const wxSize& size);
    ~CPUViewer();

    wxAuiManager auiManager;

    CDI& cdi;
    MainFrame* mainFrame;
    wxTimer renderTimer;

    wxListCtrl* internalRegisters;
    GenericList* disassembler;
    wxTextCtrl* uart;

    wxTextCtrl* d[8];
    wxTextCtrl* a[8];
    wxTextCtrl* usp;
    wxTextCtrl* ssp;
    wxTextCtrl* pc;
    wxTextCtrl* sr;

    void PaintEvent(wxPaintEvent& event);
    void PaintEvent();
    void RefreshLoop(wxTimerEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // CPUVIEWER_HPP
