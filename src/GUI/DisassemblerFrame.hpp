#ifndef DISASSEMBLERFRAME_HPP
#define DISASSEMBLERFRAME_HPP

class DisassemblerFrame;

#include <wx/textctrl.h>
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/timer.h>

#include "../Cores/SCC68070/SCC68070.hpp"
#include "MainFrame.hpp"

class DisassemblerFrame : public wxFrame
{
public:
    DisassemblerFrame(SCC68070& core, MainFrame* parent, const wxPoint& pos, const wxSize& size);
    ~DisassemblerFrame();

    SCC68070& cpu;
    MainFrame* mainFrame;
    wxPanel* registersPanel;
    wxTimer* renderTimer;

    wxTextCtrl* disassembler;
    wxTextCtrl* d0; wxTextCtrl* a0;
    wxTextCtrl* d1; wxTextCtrl* a1;
    wxTextCtrl* d2; wxTextCtrl* a2;
    wxTextCtrl* d3; wxTextCtrl* a3;
    wxTextCtrl* d4; wxTextCtrl* a4;
    wxTextCtrl* d5; wxTextCtrl* a5;
    wxTextCtrl* d6; wxTextCtrl* a6;
    wxTextCtrl* d7; wxTextCtrl* a7;
    wxTextCtrl* pc; wxTextCtrl* sr;

    void OnClose(wxCloseEvent& event);
    void PaintEvent(wxPaintEvent& event);
    void PaintEvent();
    void RefreshLoop(wxTimerEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // DISASSEMBLERFRAME_HPP
