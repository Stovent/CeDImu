#include "CPUViewer.hpp"
#include "MainFrame.hpp"
#include "../CDI/common/utils.hpp"

#include <wx/button.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/wrapsizer.h>

#include <iomanip>
#include <sstream>

static const std::array<wxString, 3> SIZES{
    "Byte",
    "Word",
    "Long",
};

static const wxArrayString REGS_SIZE{3, SIZES.data()};

static constexpr std::array<const char*, 20> REGISTER_NAMES{
    "D0",
    "D1",
    "D2",
    "D3",
    "D4",
    "D5",
    "D6",
    "D7",
    "A0",
    "A1",
    "A2",
    "A3",
    "A4",
    "A5",
    "A6",
    "A7",
    "USP",
    "SSP",
    "PC",
    "SR",
};

static wxString formatRegister(const uint32_t value, const int size, const bool hex)
{
    // TODO: std::format().
    std::stringstream stream;

    if(hex)
        stream << std::hex;
    else
        stream << std::dec;

    if(size == 0) // Byte
    {
        stream << zeroExtend<uint8_t, uint32_t>(value >> 24) << ' '
               << zeroExtend<uint8_t, uint32_t>(value >> 16) << ' '
               << zeroExtend<uint8_t, uint32_t>(value >> 8) << ' '
               << zeroExtend<uint8_t, uint32_t>(value);
    }
    else if(size == 1) // Word
    {
        stream << as<uint16_t>(value >> 16) << ' ' << as<uint16_t>(value);
    }
    else // Long
    {
        stream << value;
    }

    return stream.str();
}

static wxString srToString(const uint16_t sr)
{
    std::stringstream stream;

    if(sr & 0x8000) stream << "T "; else stream << "   ";
    if(sr & 0x2000) stream << "S "; else stream << "   ";

    stream << std::dec << bits<8, 10>(sr) << "  ";

    if(sr & 0x0010) stream << "X "; else stream << "   ";
    if(sr & 0x0008) stream << "N "; else stream << "   ";
    if(sr & 0x0004) stream << "Z "; else stream << "   ";
    if(sr & 0x0002) stream << "V "; else stream << "   ";
    if(sr & 0x0001) stream << "C "; else stream << "   ";

    return stream.str();
}

wxBEGIN_EVENT_TABLE(CPUViewer, wxFrame)
    EVT_TIMER(wxID_ANY, CPUViewer::UpdateManager)
wxEND_EVENT_TABLE()

CPUViewer::CPUViewer(MainFrame* mainFrame, CeDImu& cedimu)
    : wxFrame(mainFrame, wxID_ANY, "CPU Viewer", wxDefaultPosition, wxSize(850, 600))
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
    {
        m_internalList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);
        m_auiManager.AddPane(m_internalList, wxAuiPaneInfo().Left().Caption("Internal registers").CloseButton(false).Floatable().Resizable());

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
    }

    // Disassembler
    {
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
            if(item >= as<long>(this->m_instructions.size()))
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
    }

    // UART
    {
        m_uartTextCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
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

        m_auiManager.AddPane(m_uartTextCtrl, wxAuiPaneInfo().Bottom().Caption("UART out").CloseButton(false).Floatable().Resizable().BestSize(-1, 200));
    }

    // Control buttons
    {
        wxPanel* controlsPanel = new wxPanel(this);
        wxBoxSizer* controlsSizer = new wxBoxSizer(wxVERTICAL);

        // Execute
        wxWrapSizer* executeSizer = new wxWrapSizer(wxHORIZONTAL);

        m_executeCtrl = new wxTextCtrl(controlsPanel, wxID_ANY);
        executeSizer->Add(m_executeCtrl, wxSizerFlags().CenterVertical());

        wxButton* executeButton = new wxButton(controlsPanel, wxID_ANY, "Execute instructions");
        executeButton->Bind(wxEVT_BUTTON, [&] (wxCommandEvent&) {
            long count = 0;
            if(!m_executeCtrl->GetValue().ToLong(&count) || count < 0)
            {
                m_executeCtrl->SetValue(""); // Clear the text if invalid.
                return;
            }

            std::lock_guard<std::recursive_mutex> lock(m_cedimu.m_cdiMutex);
            if(m_cedimu.m_cdi)
                while(count--)
                    m_cedimu.m_cdi->m_cpu.Run(false);
        });
        executeSizer->Add(executeButton, wxSizerFlags().CenterVertical());

        controlsSizer->Add(executeSizer, wxSizerFlags().Border());

        // Checkbox
        m_autoscrollDisassembler = new wxCheckBox(controlsPanel, wxID_ANY, "Auto-scroll disassembler");
        m_autoscrollDisassembler->SetValue(true);
        controlsSizer->Add(m_autoscrollDisassembler, wxSizerFlags().Border());

        controlsPanel->SetSizerAndFit(controlsSizer);
        m_auiManager.AddPane(controlsPanel, wxAuiPaneInfo().Bottom().Caption("Controls"));
    }

    // CPU Registers
    {
        wxPanel* registersPanel = new wxPanel(this);
        wxFlexGridSizer* cpuRegsSizer = new wxFlexGridSizer(0, 3, 0, 0);
        cpuRegsSizer->SetFlexibleDirection(wxHORIZONTAL);
        cpuRegsSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

        for(int i = 0; i < 20; i++)
        {
            wxStaticText* text = new wxStaticText(registersPanel, wxID_ANY, REGISTER_NAMES.at(i));
            cpuRegsSizer->Add(text, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 2);

            m_registersValue[i] = new wxTextCtrl(registersPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
            cpuRegsSizer->Add(m_registersValue[i], 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);

            if(i < 8)
            {
                wxBoxSizer* regButtonsSizer = new wxBoxSizer(wxHORIZONTAL);

                m_registersSize[i] = new wxChoice(registersPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, REGS_SIZE);
                m_registersSize[i]->SetSelection(2); // long as default
                m_registersSize[i]->Bind(wxEVT_CHOICE, [&] (wxCommandEvent&) { this->UpdateCPURegisters(); });
                regButtonsSizer->Add(m_registersSize[i], wxSizerFlags().CentreVertical().Border(wxRIGHT));

                m_registersHex[i] = new wxCheckBox(registersPanel, wxID_ANY, "Hex");
                m_registersHex[i]->Bind(wxEVT_CHECKBOX, [&] (wxCommandEvent&) { this->UpdateCPURegisters(); });
                regButtonsSizer->Add(m_registersHex[i], wxSizerFlags().CentreVertical());

                cpuRegsSizer->Add(regButtonsSizer);
            }
            else if(i == 19) // SR
            {
                m_srDisassembled = new wxTextCtrl(registersPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
                cpuRegsSizer->Add(m_srDisassembled, wxSizerFlags().CenterVertical());
            }
            else
                cpuRegsSizer->Add(new wxPanel(registersPanel)); // Dummy to fill the sizer.
        }

        registersPanel->SetSizerAndFit(cpuRegsSizer);
        m_auiManager.AddPane(registersPanel, wxAuiPaneInfo().Right().Layer(1).Caption("CPU registers").CloseButton(false).Floatable().Resizable());
    }

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
    if(m_autoscrollDisassembler->IsChecked())
        m_disassemblerList->EnsureVisible(m_disassemblerList->GetItemCount() - 1);

    UpdateInternal();
    UpdateCPURegisters();
    UpdateUART();
}

void CPUViewer::UpdateInternal()
{
    std::lock_guard<std::recursive_mutex> lock(this->m_cedimu.m_cdiMutex);
    if(!m_cedimu.m_cdi)
        return;

    std::vector<InternalRegister> internal = m_cedimu.m_cdi->m_cpu.GetInternalRegisters();
    long i = 0;
    if(internal.size() != as<size_t>(m_internalList->GetItemCount()))
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

void CPUViewer::UpdateCPURegisters()
{
    std::lock_guard<std::recursive_mutex> lock(this->m_cedimu.m_cdiMutex);
    if(!m_cedimu.m_cdi)
        return;

    std::map<SCC68070::Register, uint32_t> cpuRegs = m_cedimu.m_cdi->m_cpu.GetCPURegisters();
    int i = 0;
    for(std::pair<SCC68070::Register, uint32_t> reg : cpuRegs)
    {
        if(reg.first == SCC68070::Register::SR)
        {
            m_registersValue[i++]->SetValue(toBinString(reg.second, 16));
            m_srDisassembled->SetValue(srToString(reg.second));
        }
        else
        {
            const int size = i < 8 ? m_registersSize[i]->GetSelection() : 2;
            const bool hex = i < 8 ? m_registersHex[i]->IsChecked() : true;
            m_registersValue[i++]->SetValue(formatRegister(reg.second, size, hex));
        }
    }
}

void CPUViewer::UpdateUART()
{
    std::lock_guard<std::mutex> lock(m_uartMutex);
    m_uartTextCtrl->AppendText(wxString(m_uartMissing.data(), m_uartMissing.size()));
    m_uartMissing.clear();
}
