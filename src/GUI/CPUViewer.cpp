#include "CPUViewer.hpp"
#include "enums.hpp"
#include "../utils.hpp"

#include <wx/dcclient.h>

wxBEGIN_EVENT_TABLE(CPUViewer, wxFrame)
    EVT_TIMER(wxID_ANY, CPUViewer::RefreshLoop)
    EVT_PAINT(CPUViewer::PaintEvent)
    EVT_CLOSE(CPUViewer::OnClose)
wxEND_EVENT_TABLE()

CPUViewer::CPUViewer(SCC68070* core, MainFrame* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "SCC68070 viewer", pos, size), auiManager(this), cpu(core)
{
    cpu->disassemble = true;
    mainFrame = parent;

    disassembler = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE);

    wxPanel* registersPanel = new wxPanel(this);
    for(uint8_t i = 0; i < 8; i++)
    {
        d[i] = new wxTextCtrl(registersPanel, wxID_ANY, "D" + std::to_string(i) + ": 0", wxPoint(0, i * 23), wxSize(125, 23), wxTE_READONLY); d[i]->SetBackgroundColour(*wxWHITE);
        a[i] = new wxTextCtrl(registersPanel, wxID_ANY, "A" + std::to_string(i) + ": 0", wxPoint(0, i * 23 + 189), wxSize(125, 23), wxTE_READONLY); a[i]->SetBackgroundColour(*wxWHITE);
    }
    pc = new wxTextCtrl(registersPanel, IDCPUViewerpc, "PC: 0", wxPoint(0, 377), wxSize(125, 23), wxTE_READONLY); pc->SetBackgroundColour(*wxWHITE);
    sr = new wxTextCtrl(registersPanel, IDCPUViewersr, "SR: 0", wxPoint(0, 400), wxSize(125,23), wxTE_READONLY); sr->SetBackgroundColour(*wxWHITE);

    auiManager.AddPane(disassembler, wxAuiPaneInfo().Caption("Disassembler").Center().CloseButton(false).Resizable());
    auiManager.AddPane(registersPanel, wxAuiPaneInfo().Caption("Registers").Right().CloseButton(false).Resizable().BestSize(130, 0));
    auiManager.Update();

    renderTimer = new wxTimer(this);
    renderTimer->Start(16);
}

CPUViewer::~CPUViewer()
{
    auiManager.UnInit();
    mainFrame->cpuViewer = nullptr;
    delete renderTimer;
}

void CPUViewer::OnClose(wxCloseEvent& event)
{
    renderTimer->Stop();
    Destroy();
    cpu->disassemble = false;
}

void CPUViewer::PaintEvent(wxPaintEvent& event)
{
    PaintEvent();
}

void CPUViewer::RefreshLoop(wxTimerEvent& event)
{
    PaintEvent();
}

void CPUViewer::PaintEvent()
{
    std::map<std::string, uint32_t> regs = cpu->GetRegisters();
    for(uint8_t i = 0; i < 8; i++)
    {
        d[i]->SetLabelText("D" + std::to_string(i) + ": " + std::to_string(regs["D" + std::to_string(i)]));
        a[i]->SetLabelText("A" + std::to_string(i) + ": 0x" + toHex(regs["A" + std::to_string(i)]));
    }
    pc->SetLabelText("PC: 0x" + toHex(regs["PC"]));
    sr->SetLabelText("SR: " + toBinString(regs["SR"], 16));

    instructions.str("");
    std::ostream_iterator<std::string> ssit(instructions, "\n");
    std::copy(cpu->disassembledInstructions.begin(), cpu->disassembledInstructions.end(), ssit);
    disassembler->SetValue(instructions.str());
}
