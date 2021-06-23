#include "CPUViewer.hpp"
#include "enums.hpp"
#include "../CDI/common/utils.hpp"

#include <wx/dcclient.h>

wxBEGIN_EVENT_TABLE(CPUViewer, wxFrame)
    EVT_TIMER(IDCPUViewerTimer, CPUViewer::RefreshLoop)
    EVT_PAINT(CPUViewer::PaintEvent)
wxEND_EVENT_TABLE()

CPUViewer::CPUViewer(CDI& idc, MainFrame* parent, const wxPoint& pos, const wxSize& size) :
    wxFrame(parent, wxID_ANY, "SCC68070 viewer", pos, size),
    auiManager(this),
    cdi(idc),
    renderTimer(this, IDCPUViewerTimer)
{
    mainFrame = parent;
    flushInstructions = false;

    disassembler = new GenericList(this, [=] (wxListCtrl* frame) {

        frame->EnableAlternateRowColours();

        wxListItem addressCol;
        addressCol.SetId(0);
        addressCol.SetText("Address");
        addressCol.SetWidth(70);
        frame->InsertColumn(0, addressCol);

        wxListItem kernelCol;
        kernelCol.SetId(1);
        kernelCol.SetText("Kernel location");
        kernelCol.SetWidth(100);
        frame->InsertColumn(1, kernelCol);

        wxListItem instructionCol;
        instructionCol.SetId(2);
        instructionCol.SetText("Instruction");
        instructionCol.SetWidth(700);
        frame->InsertColumn(2, instructionCol);

    }, [this] (long item, long column) -> std::string {
        if(column == 0)
            return toHex(this->instructions[item].address);
        if(column == 1)
            return this->instructions[item].biosLocation;
        return this->instructions[item].instruction;
    });

    uart = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE);

    wxPanel* registersPanel = new wxPanel(this);
    for(uint8_t i = 0; i < 8; i++)
    {
        d[i] = new wxTextCtrl(registersPanel, wxID_ANY, "D" + std::to_string(i) + ": 0", wxPoint(0, i * 23), wxSize(125, 23), wxTE_READONLY); d[i]->SetBackgroundColour(*wxWHITE);
        a[i] = new wxTextCtrl(registersPanel, wxID_ANY, "A" + std::to_string(i) + ": 0", wxPoint(0, i * 23 + 189), wxSize(125, 23), wxTE_READONLY); a[i]->SetBackgroundColour(*wxWHITE);
    }
    usp = new wxTextCtrl(registersPanel, wxID_ANY, "USP: 0", wxPoint(0, 377), wxSize(125, 23), wxTE_READONLY); usp->SetBackgroundColour(*wxWHITE);
    ssp = new wxTextCtrl(registersPanel, wxID_ANY, "SSP: 0", wxPoint(0, 400), wxSize(125, 23), wxTE_READONLY); ssp->SetBackgroundColour(*wxWHITE);
    pc = new wxTextCtrl(registersPanel, IDCPUViewerpc, "PC: 0", wxPoint(0, 423), wxSize(125, 23), wxTE_READONLY); pc->SetBackgroundColour(*wxWHITE);
    sr = new wxTextCtrl(registersPanel, IDCPUViewersr, "SR: 0", wxPoint(0, 446), wxSize(125, 23), wxTE_READONLY); sr->SetBackgroundColour(*wxWHITE);

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

    std::vector<CPUInternalRegister> iregs = cdi.board->cpu.GetInternalRegisters();
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
    auiManager.AddPane(uart, wxAuiPaneInfo().Caption("UART").Bottom().CloseButton(false).Resizable().BestSize(0, 100));
    auiManager.Update();

    uart->Bind(wxEVT_KEY_DOWN, [this] (wxKeyEvent& event) {
        const int key = event.GetKeyCode();
        if(key < 128)
            this->cdi.board->cpu.SendUARTIn(key);
    });

    cdi.callbacks.SetOnUARTOut([this] (uint8_t byte) -> void {
        this->mainFrame->app.uartOut.put((char)byte);
        this->uart->AppendText((char)byte);
    });

    cdi.callbacks.SetOnDisassembler([this] (const Instruction& inst) {
        if(this->flushInstructions)
        {
            for(const Instruction& inst : this->instructions)
                this->mainFrame->app.logInstructions << std::hex << inst.address << "\t(" << inst.biosLocation << ")\t" << inst.instruction << std::endl;

            this->instructions.clear();
            this->flushInstructions = false;
        }
        this->instructions.push_back(inst);
    });

    renderTimer.Start(16);
}

CPUViewer::~CPUViewer()
{
    cdi.callbacks.SetOnDisassembler(nullptr);
    cdi.callbacks.SetOnUARTOut(nullptr);
    mainFrame->cpuViewer = nullptr;
    renderTimer.Stop();
    auiManager.UnInit();
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
    std::map<std::string, uint32_t> regs = cdi.board->cpu.GetCPURegisters();
    for(uint8_t i = 0; i < 8; i++)
    {
        d[i]->SetLabelText("D" + std::to_string(i) + ": " + std::to_string(regs["D" + std::to_string(i)]));
        a[i]->SetLabelText("A" + std::to_string(i) + ": 0x" + toHex(regs["A" + std::to_string(i)]));
    }
    usp->SetLabelText("USP: 0x" + toHex(regs["USP"]));
    ssp->SetLabelText("SSP: 0x" + toHex(regs["SSP"]));
    pc->SetLabelText("PC: 0x" + toHex(regs["PC"]));
    sr->SetLabelText("SR: " + toBinString(regs["SR"], 16));

    disassembler->SetItemCount(instructions.size());
    disassembler->Refresh();

    std::vector<CPUInternalRegister> iregs = cdi.board->cpu.GetInternalRegisters();
    long i = 0;
    for(const CPUInternalRegister& reg : iregs)
    {
        internalRegisters->SetItem(i, 2, toHex(reg.value));
        internalRegisters->SetItem(i++, 3, reg.disassembledValue);
    }
}
