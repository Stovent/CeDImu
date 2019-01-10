#ifndef DISASSEMBLERFRAME_HPP
#define DISASSEMBLERFRAME_HPP

class DisassemblerFrame;

#include <wx/frame.h>
#include <wx/sizer.h>

#include "../Cores/SCC68070/SCC68070.hpp"
#include "MainFrame.hpp"

class DisassemblerFrame : public wxFrame
{
public:
    DisassemblerFrame(SCC68070& core, MainFrame* parent, const wxPoint& pos, const wxSize& size);
    ~DisassemblerFrame();

private:
    SCC68070& cpu;
    MainFrame* mainFrame;
    wxPanel* registersPanel;
    wxTextCtrl* disassembler;
    wxBoxSizer* sizer;

    wxTextCtrl* d0; wxTextCtrl* a0;
    wxTextCtrl* d1; wxTextCtrl* a1;
    wxTextCtrl* d2; wxTextCtrl* a2;
    wxTextCtrl* d3; wxTextCtrl* a3;
    wxTextCtrl* d4; wxTextCtrl* a4;
    wxTextCtrl* d5; wxTextCtrl* a5;
    wxTextCtrl* d6; wxTextCtrl* a6;
    wxTextCtrl* d7; wxTextCtrl* a7;
    wxTextCtrl* pc; wxTextCtrl* sr;

    void PaintEvent(wxPaintEvent& event);

private:
    static const wxEventTableEntry sm_eventTableEntries[];
protected:
    static const wxEventTable        sm_eventTable;
    virtual const wxEventTable*      GetEventTable() const;
    static wxEventHashTable          sm_eventHashTable;
    virtual wxEventHashTable&        GetEventHashTable() const;
};

#endif // DISASSEMBLERFRAME_HPP
