#include <string>

#include <wx/msgdlg.h>
#include <wx/dcclient.h>
#include <wx/textctrl.h>

#include "DisassemblerFrame.hpp"
#include "../utils.hpp"

wxBEGIN_EVENT_TABLE(DisassemblerFrame, wxFrame)
    EVT_PAINT(DisassemblerFrame::PaintEvent)
wxEND_EVENT_TABLE()

DisassemblerFrame::DisassemblerFrame(SCC68070& core, MainFrame* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "SCC68070 Disassembler", pos, size), cpu(core)
{
    mainFrame = parent;

    disassembler = new wxTextCtrl(this, wxID_ANY, "abc", wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE);

    registersPanel = new wxPanel(this);
    d0 = new wxTextCtrl(registersPanel, IDDisassemblerd0, "", wxPoint(0, 00), wxDefaultSize, wxTE_READONLY);   a0 = new wxTextCtrl(registersPanel, IDDisassemblera0, "", wxPoint(111, 00), wxDefaultSize, wxTE_READONLY);
    d1 = new wxTextCtrl(registersPanel, IDDisassemblerd1, "", wxPoint(0, 23), wxDefaultSize, wxTE_READONLY);   a1 = new wxTextCtrl(registersPanel, IDDisassemblera1, "", wxPoint(111, 23), wxDefaultSize, wxTE_READONLY);
    d2 = new wxTextCtrl(registersPanel, IDDisassemblerd2, "", wxPoint(0, 46), wxDefaultSize, wxTE_READONLY);   a2 = new wxTextCtrl(registersPanel, IDDisassemblera2, "", wxPoint(111, 46), wxDefaultSize, wxTE_READONLY);
    d3 = new wxTextCtrl(registersPanel, IDDisassemblerd3, "", wxPoint(0, 69), wxDefaultSize, wxTE_READONLY);   a3 = new wxTextCtrl(registersPanel, IDDisassemblera3, "", wxPoint(111, 69), wxDefaultSize, wxTE_READONLY);
    d4 = new wxTextCtrl(registersPanel, IDDisassemblerd4, "", wxPoint(0, 92), wxDefaultSize, wxTE_READONLY);   a4 = new wxTextCtrl(registersPanel, IDDisassemblera4, "", wxPoint(111, 92), wxDefaultSize, wxTE_READONLY);
    d5 = new wxTextCtrl(registersPanel, IDDisassemblerd5, "", wxPoint(0, 115), wxDefaultSize, wxTE_READONLY);  a5 = new wxTextCtrl(registersPanel, IDDisassemblera5, "", wxPoint(111, 115), wxDefaultSize, wxTE_READONLY);
    d6 = new wxTextCtrl(registersPanel, IDDisassemblerd6, "", wxPoint(0, 138), wxDefaultSize, wxTE_READONLY);  a6 = new wxTextCtrl(registersPanel, IDDisassemblera6, "", wxPoint(111, 138), wxDefaultSize, wxTE_READONLY);
    d7 = new wxTextCtrl(registersPanel, IDDisassemblerd7, "", wxPoint(0, 162), wxDefaultSize, wxTE_READONLY);  a7 = new wxTextCtrl(registersPanel, IDDisassemblera7, "", wxPoint(111, 162), wxDefaultSize, wxTE_READONLY);
    pc = new wxTextCtrl(registersPanel, IDDisassemblerdpc, "", wxPoint(0, 185), wxDefaultSize, wxTE_READONLY); sr = new wxTextCtrl(registersPanel, IDDisassemblerasr, "", wxPoint(111, 185), wxDefaultSize, wxTE_READONLY);
    d0->SetBackgroundColour(*wxWHITE); d0->SetLabelText("D0: " + std::to_string(cpu.D[0])); a0->SetBackgroundColour(*wxWHITE); a0->SetLabelText("A0: " + std::to_string(cpu.A[0]));
    d1->SetBackgroundColour(*wxWHITE); d1->SetLabelText("D1: " + std::to_string(cpu.D[1])); a1->SetBackgroundColour(*wxWHITE); a1->SetLabelText("A1: " + std::to_string(cpu.A[1]));
    d2->SetBackgroundColour(*wxWHITE); d2->SetLabelText("D2: " + std::to_string(cpu.D[2])); a2->SetBackgroundColour(*wxWHITE); a2->SetLabelText("A2: " + std::to_string(cpu.A[2]));
    d3->SetBackgroundColour(*wxWHITE); d3->SetLabelText("D3: " + std::to_string(cpu.D[3])); a3->SetBackgroundColour(*wxWHITE); a3->SetLabelText("A3: " + std::to_string(cpu.A[3]));
    d4->SetBackgroundColour(*wxWHITE); d4->SetLabelText("D4: " + std::to_string(cpu.D[4])); a4->SetBackgroundColour(*wxWHITE); a4->SetLabelText("A4: " + std::to_string(cpu.A[4]));
    d5->SetBackgroundColour(*wxWHITE); d5->SetLabelText("D5: " + std::to_string(cpu.D[5])); a5->SetBackgroundColour(*wxWHITE); a5->SetLabelText("A5: " + std::to_string(cpu.A[5]));
    d6->SetBackgroundColour(*wxWHITE); d6->SetLabelText("D6: " + std::to_string(cpu.D[6])); a6->SetBackgroundColour(*wxWHITE); a6->SetLabelText("A6: " + std::to_string(cpu.A[6]));
    d7->SetBackgroundColour(*wxWHITE); d7->SetLabelText("D7: " + std::to_string(cpu.D[7])); a7->SetBackgroundColour(*wxWHITE); a7->SetLabelText("A7: " + std::to_string(cpu.A[7]));
    pc->SetBackgroundColour(*wxWHITE); d7->SetLabelText("PC: " + std::to_string(cpu.PC));   sr->SetBackgroundColour(*wxWHITE); sr->SetLabelText("SR: " + std::to_string(cpu.SR));

    sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(disassembler, 1, wxEXPAND);
    sizer->Add(registersPanel, 1);
    SetSizer(sizer);
}

DisassemblerFrame::~DisassemblerFrame()
{
    mainFrame->disassemblerFrame = nullptr;
}

void DisassemblerFrame::PaintEvent(wxPaintEvent& event)
{
    d0->SetLabelText("D0: " + std::to_string(cpu.D[0])); a0->SetLabelText("A0: " + std::to_string(cpu.A[0]));
    d1->SetLabelText("D1: " + std::to_string(cpu.D[1])); a1->SetLabelText("A1: " + std::to_string(cpu.A[1]));
    d2->SetLabelText("D2: " + std::to_string(cpu.D[2])); a2->SetLabelText("A2: " + std::to_string(cpu.A[2]));
    d3->SetLabelText("D3: " + std::to_string(cpu.D[3])); a3->SetLabelText("A3: " + std::to_string(cpu.A[3]));
    d4->SetLabelText("D4: " + std::to_string(cpu.D[4])); a4->SetLabelText("A4: " + std::to_string(cpu.A[4]));
    d5->SetLabelText("D5: " + std::to_string(cpu.D[5])); a5->SetLabelText("A5: " + std::to_string(cpu.A[5]));
    d6->SetLabelText("D6: " + std::to_string(cpu.D[6])); a6->SetLabelText("A6: " + std::to_string(cpu.A[6]));
    d7->SetLabelText("D7: " + std::to_string(cpu.D[7])); a7->SetLabelText("A7: " + std::to_string(cpu.A[7]));
    pc->SetLabelText("PC: " + std::to_string(cpu.PC));   sr->SetLabelText("SR: " + toBinString(cpu.SR, 16));

    wxPaintDC dc(this);
    dc.DrawText(std::to_string(cpu.run), 50, 50);
}
