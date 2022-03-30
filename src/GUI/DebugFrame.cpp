#include "DebugFrame.hpp"
#include "MainFrame.hpp"
#include "../CDI/common/utils.hpp"

#include "wx/panel.h"
#include "wx/sizer.h"

wxBEGIN_EVENT_TABLE(DebugFrame, wxFrame)
    EVT_TIMER(wxID_ANY, DebugFrame::UpdateManager)
wxEND_EVENT_TABLE()

DebugFrame::DebugFrame(MainFrame* mainFrame, CeDImu& cedimu) :
    wxFrame(mainFrame, wxID_ANY, "Debug", wxDefaultPosition, wxSize(600, 500)),
    m_cedimu(cedimu),
    m_mainFrame(mainFrame),
    m_auiManager(this),
    m_updateTimer(this, wxID_ANY),
    m_updateMemoryLogs(false),
    m_trapCount(0),
    m_updateExceptions(false)
{
    // Memory logs
    wxPanel* memoryPanel = new wxPanel(this);
    m_auiManager.AddPane(memoryPanel, wxAuiPaneInfo().Center().Caption("Memory access").CloseButton(false).Floatable().Resizable());
    wxBoxSizer* memoryPanelSizer = new wxBoxSizer(wxVERTICAL);
    memoryPanel->SetSizer(memoryPanelSizer);

    wxBoxSizer* memoryButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    memoryPanelSizer->Add(memoryButtonsSizer, wxSizerFlags().Expand());
    m_logCpu = new wxCheckBox(memoryPanel, wxID_ANY, "CPU");
    memoryButtonsSizer->Add(m_logCpu, wxSizerFlags().Proportion(1));
    m_logBios = new wxCheckBox(memoryPanel, wxID_ANY, "BIOS");
    memoryButtonsSizer->Add(m_logBios, wxSizerFlags().Proportion(1));
    m_logRam = new wxCheckBox(memoryPanel, wxID_ANY, "RAM");
    memoryButtonsSizer->Add(m_logRam, wxSizerFlags().Proportion(1));
    m_logVdsc = new wxCheckBox(memoryPanel, wxID_ANY, "VDSC");
    memoryButtonsSizer->Add(m_logVdsc, wxSizerFlags().Proportion(1));
    m_logSlave = new wxCheckBox(memoryPanel, wxID_ANY, "Slave");
    memoryButtonsSizer->Add(m_logSlave, wxSizerFlags().Proportion(1));
    m_logCdic = new wxCheckBox(memoryPanel, wxID_ANY, "CDIC");
    memoryButtonsSizer->Add(m_logCdic, wxSizerFlags().Proportion(1));
    m_logNvram = new wxCheckBox(memoryPanel, wxID_ANY, "NVRAM");
    memoryButtonsSizer->Add(m_logNvram, wxSizerFlags().Proportion(1));
    m_logOutOfRange = new wxCheckBox(memoryPanel, wxID_ANY, "Out of range");
    memoryButtonsSizer->Add(m_logOutOfRange, wxSizerFlags().Proportion(1));

    m_memoryLogsList = new GenericList(memoryPanel, [=] (wxListCtrl* list) {
        wxListItem location;
        location.SetText("Location");
        location.SetWidth(60);
        list->InsertColumn(0, location);

        wxListItem pc;
        pc.SetText("PC");
        pc.SetWidth(60);
        list->InsertColumn(1, pc);

        wxListItem direction;
        direction.SetText("Direction");
        direction.SetWidth(60);
        list->InsertColumn(2, direction);

        wxListItem type;
        type.SetText("Type");
        type.SetWidth(60);
        list->InsertColumn(3, type);

        wxListItem address;
        address.SetText("Address");
        address.SetWidth(65);
        list->InsertColumn(4, address);

        wxListItem value;
        value.SetText("Value");
        value.SetWidth(60);
        list->InsertColumn(5, value);

        wxListItem valueHex;
        valueHex.SetText("Value Hex");
        valueHex.SetWidth(65);
        list->InsertColumn(6, valueHex);
    }, [=] (long item, long column) -> wxString {
        std::lock_guard<std::mutex> lock(this->m_memoryLogsMutex);
        if(item >= (long)this->m_memoryLogs.size())
            return "";

        const LogMemoryAccess& log = this->m_memoryLogs[item];
        switch(column)
        {
        case 0:
            return memoryAccessLocationToString(log.location);
        case 1:
            return toHex(log.pc);
        case 2:
            return log.direction;
        case 3:
            return log.type;
        case 4:
            return toHex(log.address);
        case 5:
            return std::to_string(log.data);
        case 6:
            return toHex(log.data);
        default:
            return "";
        }
    });
    memoryPanelSizer->Add(m_memoryLogsList, wxSizerFlags().Expand().Proportion(1));

    m_cedimu.m_cdi.callbacks.SetOnLogMemoryAccess([=] (const LogMemoryAccess& log) {
        if((log.location == MemoryAccessLocation::CPU   && this->m_logCpu->GetValue())   ||
           (log.location == MemoryAccessLocation::BIOS  && this->m_logBios->GetValue())  ||
           (log.location == MemoryAccessLocation::RAM   && this->m_logRam->GetValue())   ||
           (log.location == MemoryAccessLocation::VDSC  && this->m_logVdsc->GetValue())  ||
           (log.location == MemoryAccessLocation::Slave && this->m_logSlave->GetValue()) ||
           (log.location == MemoryAccessLocation::CDIC  && this->m_logCdic->GetValue())  ||
           (log.location == MemoryAccessLocation::RTC   && this->m_logNvram->GetValue()) ||
           (log.location == MemoryAccessLocation::OutOfRange && this->m_logOutOfRange->GetValue()))
        {
            std::lock_guard<std::mutex> lock(this->m_memoryLogsMutex);
            this->m_memoryLogs.push_back(log);
            this->m_updateMemoryLogs = true;
            LOG(m_cedimu.WriteMemoryAccess(log);)
        }
    });


    // Exceptions
    m_exceptionsList = new GenericList(this, [=] (wxListCtrl* list) {
        wxListItem address;
        address.SetText("Return address");
        address.SetWidth(60);
        list->InsertColumn(0, address);

        wxListItem module;
        module.SetText("Module");
        module.SetWidth(60);
        list->InsertColumn(1, module);

        wxListItem text;
        text.SetText("Exception");
        text.SetWidth(210);
        list->InsertColumn(2, text);

        wxListItem syscall;
        syscall.SetText("System call");
        syscall.SetWidth(80);
        list->InsertColumn(3, syscall);

        wxListItem inputs;
        inputs.SetText("System call inputs");
        inputs.SetWidth(450);
        list->InsertColumn(4, inputs);

        wxListItem outputs;
        outputs.SetText("System call outputs");
        outputs.SetWidth(400);
        list->InsertColumn(5, outputs);
    }, [=] (long item, long column) -> wxString {
        std::lock_guard<std::mutex> lock(this->m_exceptionsMutex);
        if(item >= (long)this->m_exceptions.size())
            return "";

        const std::pair<size_t, LogSCC68070Exception>& log = this->m_exceptions[this->m_exceptions.size() - 1 - item];
        if(column == 0)
            return toHex(log.second.returnAddress);
        if(column == 1 && log.second.vector == SCC68070::Trap0Instruction)
            return log.second.systemCall.module;
        if(column == 2)
            return log.second.disassembled;
        if(column == 3 && log.second.vector == SCC68070::Trap0Instruction)
            return OS9::systemCallNameToString(log.second.systemCall.type);
        if(column == 4)
            return log.second.systemCall.inputs;
        if(column == 5)
            return log.second.systemCall.outputs;
        return "";
    });
    m_auiManager.AddPane(m_exceptionsList, wxAuiPaneInfo().Bottom().Caption("Exceptions stack").CloseButton(false).Floatable().Resizable());

    m_cedimu.m_cdi.callbacks.SetOnLogException([=] (const LogSCC68070Exception& log) {
        std::lock_guard<std::mutex> lock(m_exceptionsMutex);
        m_updateExceptions = true;
        const size_t trap = log.vector >= SCC68070::Trap0Instruction && log.vector <= SCC68070::Trap15Instruction ? ++m_trapCount : 0;
        m_exceptions.push_back({trap, log});
        LOG(m_cedimu.WriteException(log, trap);)
    });

    m_cedimu.m_cdi.callbacks.SetOnLogRTE([=] (uint32_t pc, uint16_t format) {
        std::lock_guard<std::mutex> lock(m_exceptionsMutex);
        m_updateExceptions = true;
        for(std::vector<std::pair<size_t, LogSCC68070Exception>>::reverse_iterator it = this->m_exceptions.rbegin();
            it != this->m_exceptions.rend(); it++)
        {
            if(pc == it->second.returnAddress && it->second.vector == SCC68070::Trap0Instruction)
            {
                size_t index = it->first;
                const std::map<SCC68070::Register, uint32_t> registers = m_cedimu.m_cdi.board->cpu.GetCPURegisters();
                const bool cc = registers.at(SCC68070::Register::SR) & 1;
                char error[64] = {0};
                std::string outputs;
                m_updateExceptions = true;

                if(cc)
                {
                    snprintf(error, 64, "carry=%d d1.w=%s ", cc, OS9::errorNameToString(OS9::Error(registers.at(SCC68070::Register::D1))).c_str());
                }
                else
                {
                    outputs = OS9::systemCallOutputsToString(it->second.systemCall.type, registers, [=] (const uint32_t addr) -> const uint8_t* { return this->m_cedimu.m_cdi.board->GetPointer(addr); });
                }

                const OS9::SystemCall syscall = {it->second.systemCall.type, "", "", cc ? std::string(error) : outputs};
                const LogSCC68070Exception rte{it->second.vector, it->second.returnAddress, it->second.disassembled, syscall};
                m_exceptions.push_back({index, rte});
                LOG(m_cedimu.WriteRTE(pc, format, rte, index);)
                break;
            }
        }
    });


    m_auiManager.Update();
    Show();
    m_updateTimer.Start(16);
}

DebugFrame::~DebugFrame()
{
    m_cedimu.m_cdi.callbacks.SetOnLogMemoryAccess(nullptr);
    m_cedimu.m_cdi.callbacks.SetOnLogException(nullptr);
    m_cedimu.m_cdi.callbacks.SetOnLogRTE(nullptr);
    m_auiManager.UnInit();
    m_mainFrame->m_debugFrame = nullptr;
}

void DebugFrame::UpdateManager(wxTimerEvent&)
{
    if(m_updateExceptions)
    {
        m_exceptionsList->SetItemCount(m_exceptions.size());
        m_exceptionsList->Refresh();
        m_updateExceptions = false;
    }

    if(m_updateMemoryLogs)
    {
        m_memoryLogsList->SetItemCount(m_memoryLogs.size());
        m_memoryLogsList->Refresh();
        m_updateMemoryLogs = false;
    }
}
