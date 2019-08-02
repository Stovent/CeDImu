#include <string>

#include <wx/msgdlg.h>
#include <wx/dcclient.h>

#include "DisassemblerFrame.hpp"
#include "../utils.hpp"

wxBEGIN_EVENT_TABLE(DisassemblerFrame, wxFrame)
    EVT_TIMER(wxID_ANY, DisassemblerFrame::RefreshLoop)
    EVT_PAINT(DisassemblerFrame::PaintEvent)
    EVT_CLOSE(DisassemblerFrame::OnClose)
wxEND_EVENT_TABLE()

DisassemblerFrame::DisassemblerFrame(SCC68070& core, MainFrame* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "SCC68070 Disassembler", pos, size), cpu(core)
{
    cpu.disassemble = true;
    currentRow = 0;
    mainFrame = parent;

    disassembler = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE);

    wxPanel* registersPanel = new wxPanel(this);
    for(uint8_t i = 0; i < 8; i++)
    {
        d[i] = new wxTextCtrl(registersPanel, wxID_ANY, "D" + std::to_string(i) + ": " + std::to_string(cpu.D[i]), wxPoint(0, i * 23), wxSize(125, 23), wxTE_READONLY); d[i]->SetBackgroundColour(*wxWHITE);
        a[i] = new wxTextCtrl(registersPanel, wxID_ANY, "A" + std::to_string(i) + ": " + toHex(cpu.A[i]),    wxPoint(0, i * 23 + 189), wxSize(125, 23), wxTE_READONLY); a[i]->SetBackgroundColour(*wxWHITE);
    }
    pc = new wxTextCtrl(registersPanel, IDDisassemblerpc, "PC: " + toHex(cpu.PC), wxPoint(0, 377), wxSize(125, 23), wxTE_READONLY); pc->SetBackgroundColour(*wxWHITE);
    sr = new wxTextCtrl(registersPanel, IDDisassemblersr, "SR: " + toBinString(cpu.SR, 16), wxPoint(0, 400), wxSize(125,23), wxTE_READONLY); sr->SetBackgroundColour(*wxWHITE);

    wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(disassembler, 1, wxEXPAND);
    sizer->Add(registersPanel, 0, wxEXPAND | wxALIGN_RIGHT);
    SetSizer(sizer);

    renderTimer = new wxTimer(this);
    renderTimer->Start(16);
}

DisassemblerFrame::~DisassemblerFrame()
{
    mainFrame->disassemblerFrame = nullptr;
    delete renderTimer;
}

void DisassemblerFrame::OnClose(wxCloseEvent& event)
{
    renderTimer->Stop();
    Destroy();
    cpu.disassemble = false;
}

void DisassemblerFrame::PaintEvent(wxPaintEvent& event)
{
    PaintEvent();
}

void DisassemblerFrame::RefreshLoop(wxTimerEvent& event)
{
    PaintEvent();
}

void DisassemblerFrame::PaintEvent()
{
    if(cpu.instructionsBufferChanged)
    {
        for(uint8_t i = 0; i < 8; i++)
        {
            d[i]->SetLabelText("D" + std::to_string(i) + ": " + std::to_string(cpu.D[i]));
            a[i]->SetLabelText("A" + std::to_string(i) + ": 0x" + toHex(cpu.A[i]));
        }
        pc->SetLabelText("PC: 0x" + toHex(cpu.PC));
        sr->SetLabelText("SR: " + toBinString(cpu.SR, 16));

        if(cpu.app.mainFrame->pause->IsChecked())
        {
            if(currentRow > cpu.instructionsBuffer.size())
            {
                disassembler->SetLabelText("");
                currentRow = 0;
            }

            for(; currentRow < cpu.instructionsBuffer.size(); currentRow++)
                disassembler->AppendText(cpu.instructionsBuffer[currentRow] + "\n");
        }
        cpu.instructionsBufferChanged = false;
    }
}
