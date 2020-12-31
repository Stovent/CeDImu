#include "CPUViewer.hpp"
#include "enums.hpp"
#include "../CDI/common/utils.hpp"

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
    uartOut = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE);

    wxPanel* registersPanel = new wxPanel(this);
    for(uint8_t i = 0; i < 8; i++)
    {
        d[i] = new wxTextCtrl(registersPanel, wxID_ANY, "D" + std::to_string(i) + ": 0", wxPoint(0, i * 23), wxSize(125, 23), wxTE_READONLY); d[i]->SetBackgroundColour(*wxWHITE);
        a[i] = new wxTextCtrl(registersPanel, wxID_ANY, "A" + std::to_string(i) + ": 0", wxPoint(0, i * 23 + 189), wxSize(125, 23), wxTE_READONLY); a[i]->SetBackgroundColour(*wxWHITE);
    }
    pc = new wxTextCtrl(registersPanel, IDCPUViewerpc, "PC: 0", wxPoint(0, 377), wxSize(125, 23), wxTE_READONLY); pc->SetBackgroundColour(*wxWHITE);
    sr = new wxTextCtrl(registersPanel, IDCPUViewersr, "SR: 0", wxPoint(0, 400), wxSize(125, 23), wxTE_READONLY); sr->SetBackgroundColour(*wxWHITE);

    internalRegisters = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(300, 0), wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);
    wxListItem nameCol;
    nameCol.SetId(0);
    nameCol.SetText("Name");
    nameCol.SetWidth(60);
    internalRegisters->InsertColumn(0, nameCol);

    wxListItem addressCol;
    addressCol.SetId(1);
    addressCol.SetText("Address");
    addressCol.SetWidth(62);
    internalRegisters->InsertColumn(1, addressCol);

    wxListItem valueCol;
    valueCol.SetId(2);
    valueCol.SetText("Value");
    valueCol.SetWidth(60);
    internalRegisters->InsertColumn(2, valueCol);

    wxListItem disCol;
    disCol.SetId(3);
    disCol.SetText("Disassembled value");
    disCol.SetWidth(120);
    internalRegisters->InsertColumn(3, disCol);

    std::vector<CPUInternalRegister> iregs = cpu->GetInternalRegisters();
    long i = 0;
    for(const CPUInternalRegister& reg : iregs)
    {
        long itemIndex = internalRegisters->InsertItem(i++, reg.name);
        internalRegisters->SetItem(itemIndex, 1, toHex(reg.address));
        internalRegisters->SetItem(itemIndex, 2, toHex(reg.value));
        internalRegisters->SetItem(itemIndex, 3, reg.disassembledValue);
    }

    auiManager.AddPane(disassembler, wxAuiPaneInfo().Caption("Disassembler").Center().CloseButton(false).Resizable());
    auiManager.AddPane(internalRegisters, wxAuiPaneInfo().Caption("Internal Registers").Left().CloseButton(false).Resizable().BestSize(300, 0));
    auiManager.AddPane(registersPanel, wxAuiPaneInfo().Caption("CPU Registers").Right().CloseButton(false).Resizable().BestSize(130, 0));
    auiManager.AddPane(uartOut, wxAuiPaneInfo().Caption("UART Output").Bottom().CloseButton(false).Resizable().BestSize(0, 100));
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
    std::map<std::string, uint32_t> regs = cpu->GetCPURegisters();
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

    std::vector<CPUInternalRegister> iregs = cpu->GetInternalRegisters();
    long i = 0;
    for(const CPUInternalRegister& reg : iregs)
    {
        internalRegisters->SetItem(i, 2, toHex(reg.value));
        internalRegisters->SetItem(i++, 3, reg.disassembledValue);
    }
}
