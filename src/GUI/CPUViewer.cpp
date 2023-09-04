#include "CPUViewer.hpp"
#include "MainFrame.hpp"
#include "../CDI/common/utils.hpp"

#include <wx/panel.h>
#include <wx/sizer.h>

wxBEGIN_EVENT_TABLE(CPUViewer, wxFrame)
    EVT_TIMER(wxID_ANY, CPUViewer::UpdateManager)
wxEND_EVENT_TABLE()

CPUViewer::CPUViewer(MainFrame* mainFrame, CeDImu& cedimu)
    : wxFrame(mainFrame, wxID_ANY, "CPU Viewer", wxDefaultPosition, wxSize(800, 550))
    , m_cedimu(cedimu)
    , m_mainFrame(mainFrame)
    , m_auiManager(this)
    , m_updateTimer(this, wxID_ANY)
    , m_updateManager(false)
    , m_flushInstructions(false)
    , m_uartMutex()
    , m_uartMissing()
    , m_lastByte(0)
{
    // Internal registers
    m_internalList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);
    m_auiManager.AddPane(m_internalList, wxAuiPaneInfo().Left().Caption("Internal registers").CloseButton(false).Floatable().Resizable().BestSize(190, -1));

    wxListItem nameCol;
    nameCol.SetId(0);
    nameCol.SetText("Name");
    nameCol.SetWidth(60);
    m_internalList->InsertColumn(0, nameCol);

    wxListItem addressCol;
    addressCol.SetId(1);
    addressCol.SetText("Address");
    addressCol.SetWidth(62);
    m_internalList->InsertColumn(1, addressCol);

    wxListItem valueCol;
    valueCol.SetId(2);
    valueCol.SetText("Value");
    valueCol.SetWidth(45);
    m_internalList->InsertColumn(2, valueCol);

    wxListItem disCol;
    disCol.SetId(3);
    disCol.SetText("Disassembled value");
    disCol.SetWidth(120);
    m_internalList->InsertColumn(3, disCol);


    // Disassembler
    m_disassemblerList = new GenericList(this, [] (wxListCtrl* list) {
        list->EnableAlternateRowColours();

        wxListItem addressCol;
        addressCol.SetId(0);
        addressCol.SetText("Address");
        addressCol.SetWidth(70);
        list->InsertColumn(0, addressCol);

        wxListItem kernelCol;
        kernelCol.SetId(1);
        kernelCol.SetText("Location");
        kernelCol.SetWidth(70);
        list->InsertColumn(1, kernelCol);

        wxListItem instructionCol;
        instructionCol.SetId(2);
        instructionCol.SetText("Instruction");
        instructionCol.SetWidth(700);
        list->InsertColumn(2, instructionCol);
    }, [&] (long item, long column) -> wxString {
        std::lock_guard<std::mutex> lock(this->m_instructionsMutex);
        if(item >= (long)this->m_instructions.size())
            return "";
        const LogInstruction& inst = this->m_instructions[item];
        switch(column)
        {
        case 0: return toHex(inst.address);
        case 1: return inst.biosLocation;
        case 2: return inst.instruction;
        default: return "";
        }
    });
    m_auiManager.AddPane(m_disassemblerList, wxAuiPaneInfo().Center().Caption("Disassembler").CloseButton(false).Floatable().Resizable());

    m_cedimu.SetOnLogDisassembler([&] (const LogInstruction& inst) {
        std::lock_guard<std::mutex> lock(this->m_instructionsMutex);
        if(this->m_flushInstructions)
        {
            this->m_flushInstructions = false;
            this->m_instructions.clear();
        }
        this->m_instructions.push_back(inst);
        this->m_updateManager = true;
        LOG(m_cedimu.WriteInstruction(inst);)
    });


    // Registers
    wxPanel* registersPanel = new wxPanel(this);
    m_auiManager.AddPane(registersPanel, wxAuiPaneInfo().Right().Layer(1).Caption("CPU registers").CloseButton(false).Floatable().Resizable().BestSize(130, -1));
    wxBoxSizer* registersPanelSizer = new wxBoxSizer(wxVERTICAL);
    registersPanel->SetSizer(registersPanelSizer);
    for(int i = 0; i < 20; i++)
    {
        m_registers[i] = new wxTextCtrl(registersPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(130, 23), wxTE_READONLY);
        if(i == 8)
            registersPanelSizer->Add(m_registers[i], wxSizerFlags().Border(wxTOP, 10));
        else if(i == 15)
            registersPanelSizer->Add(m_registers[i], wxSizerFlags().Border(wxBOTTOM, 10));
        else
            registersPanelSizer->Add(m_registers[i]);
    }


    // UART
    m_uartTextCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    m_auiManager.AddPane(m_uartTextCtrl, wxAuiPaneInfo().Bottom().Caption("UART out").CloseButton(false).Floatable().Resizable().BestSize(-1, 200));
    m_uartTextCtrl->Bind(wxEVT_KEY_DOWN, [&] (wxKeyEvent& event) {
        const int key = event.GetKeyCode();
        std::lock_guard<std::recursive_mutex> lock(this->m_cedimu.m_cdiMutex);
        if(key < 128)
            if(this->m_cedimu.m_cdi)
                this->m_cedimu.m_cdi->m_cpu.SendUARTIn(key);
    });

    m_cedimu.SetOnUARTOut([&] (uint8_t d) {
        this->m_cedimu.m_uartOut.put(d);
        if(!((this->m_lastByte == '\r' && d == '\n') || (this->m_lastByte == '\n' && d == '\r')))
        {
            std::lock_guard<std::mutex> lock(m_uartMutex);
            m_uartMissing.push_back(d);
        }
        this->m_lastByte = d;
    });


    m_auiManager.Update();
    Show();
    m_updateTimer.Start(16);
}

CPUViewer::~CPUViewer()
{
    m_cedimu.SetOnLogDisassembler(nullptr);
    m_cedimu.SetOnUARTOut(nullptr);
    m_auiManager.UnInit();
    m_mainFrame->m_cpuViewer = nullptr;
}

void CPUViewer::UpdateManager(wxTimerEvent&)
{
    if(!m_updateManager)
        return;
    m_updateManager = false;
    m_disassemblerList->SetItemCount(m_instructions.size());
    m_disassemblerList->Refresh();
    UpdateInternal();
    UpdateRegisters();
    UpdateUART();
}

void CPUViewer::UpdateInternal()
{
    std::lock_guard<std::recursive_mutex> lock(this->m_cedimu.m_cdiMutex);
    if(!m_cedimu.m_cdi)
        return;

    std::vector<InternalRegister> internal = m_cedimu.m_cdi->m_cpu.GetInternalRegisters();
    long i = 0;
    if(internal.size() != (size_t)m_internalList->GetItemCount())
    {
        m_internalList->DeleteAllItems();
        for(const InternalRegister& reg : internal)
        {
            m_internalList->InsertItem(i++, reg.name);
        }
    }
    else
    {
        for(const InternalRegister& reg : internal)
        {
            m_internalList->SetItem(i, 1, toHex(reg.address));
            m_internalList->SetItem(i, 2, toHex(reg.value));
            m_internalList->SetItem(i, 3, reg.disassembledValue);
            i++;
        }
    }
}

void CPUViewer::UpdateRegisters()
{
    std::lock_guard<std::recursive_mutex> lock(this->m_cedimu.m_cdiMutex);
    if(!m_cedimu.m_cdi)
        return;

    const m68000_registers_t* cpuRegs = m_cedimu.m_cdi->m_cpu.CPURegisters();
    size_t reg = 0;

    for(int i = 0; i < 8; i++)
    {
        char str[64] = {0};
        snprintf(str, 64, "D%d : %d", i, cpuRegs->d[i]);
        m_registers[reg++]->SetValue(str);
    }

    for(int i = 0; i < 8; i++)
    {
        char str[64] = {0};
        if(i < 7)
            snprintf(str, 64, "A%d : 0x%X", i, cpuRegs->a[i]);
        else
            if(cpuRegs->sr.s)
                snprintf(str, 64, "A%d : 0x%X", i, cpuRegs->ssp);
            else
                snprintf(str, 64, "A%d : 0x%X", i, cpuRegs->usp);

        m_registers[reg++]->SetValue(str);
    }

    m_registers[reg++]->SetValue("PC : " + toHex(cpuRegs->pc));
    m_registers[reg++]->SetValue("SR : " + std::to_string(cpuRegs->sr.t) + " "
                                        + std::to_string(cpuRegs->sr.s) + " "
                                        + std::to_string(cpuRegs->sr.interrupt_mask) + " "
                                        + std::to_string(cpuRegs->sr.x) + " "
                                        + std::to_string(cpuRegs->sr.n) + " "
                                        + std::to_string(cpuRegs->sr.z) + " "
                                        + std::to_string(cpuRegs->sr.v) + " "
                                        + std::to_string(cpuRegs->sr.c));
    m_registers[reg++]->SetValue("SSP : " + toHex(cpuRegs->ssp));
    m_registers[reg++]->SetValue("USP : " + toHex(cpuRegs->usp));
}

void CPUViewer::UpdateUART()
{
    std::lock_guard<std::mutex> lock(m_uartMutex);
    m_uartTextCtrl->AppendText(wxString(m_uartMissing.data(), m_uartMissing.size()));
    m_uartMissing.clear();
}
