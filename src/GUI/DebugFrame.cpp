#include "DebugFrame.hpp"
#include "MainFrame.hpp"
#include "../CDI/common/utils.hpp"

#include "wx/panel.h"
#include "wx/sizer.h"

wxBEGIN_EVENT_TABLE(DebugFrame, wxFrame)
    EVT_TIMER(wxID_ANY, DebugFrame::UpdateManager)
wxEND_EVENT_TABLE()

DebugFrame::DebugFrame(MainFrame* mainFrame, CeDImu& cedimu) :
    wxFrame(mainFrame, wxID_ANY, "Debug", wxDefaultPosition, wxSize(465, 500)),
    m_cedimu(cedimu),
    m_mainFrame(mainFrame),
    m_auiManager(this),
    m_updateTimer(this, wxID_ANY),
    m_updateMemoryLogs(false)
{
    // Memory logs
    wxPanel* memoryPanel = new wxPanel(this);
    m_auiManager.AddPane(memoryPanel, wxAuiPaneInfo().Left().Caption("Memory access").CloseButton(false).Floatable().Resizable());
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
    m_logNvram = new wxCheckBox(memoryPanel, wxID_ANY, "NVRAM");
    memoryButtonsSizer->Add(m_logNvram, wxSizerFlags().Proportion(1));

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
        if((log.location == CPU   && this->m_logCpu->GetValue())   ||
           (log.location == BIOS  && this->m_logBios->GetValue())  ||
           (log.location == RAM   && this->m_logRam->GetValue())   ||
           (log.location == VDSC  && this->m_logVdsc->GetValue())  ||
           (log.location == Slave && this->m_logSlave->GetValue()) ||
           (log.location == RTC   && this->m_logNvram->GetValue())
           )
        {
            std::lock_guard<std::mutex> lock(this->m_memoryLogsMutex);
            this->m_memoryLogs.push_back(log);
            this->m_updateMemoryLogs = true;
        }
    });


    Show();
    m_updateTimer.Start(16);
}

DebugFrame::~DebugFrame()
{
    m_cedimu.m_cdi.callbacks.SetOnLogMemoryAccess(nullptr);
    m_auiManager.UnInit();
    m_mainFrame->m_debugFrame = nullptr;
}

void DebugFrame::UpdateManager(wxTimerEvent&)
{
    if(m_updateMemoryLogs)
    {
        UpdateMemoryLogs();
        m_updateMemoryLogs = false;
    }
}

void DebugFrame::UpdateMemoryLogs()
{
    m_memoryLogsList->SetItemCount(m_memoryLogs.size());
    m_memoryLogsList->Refresh();

}
