#include "DebugFrame.hpp"
#include "MainFrame.hpp"
#include "../CDI/common/utils.hpp"

#include <wx/panel.h>
#include <wx/msgdlg.h>

#include <iomanip>

wxBEGIN_EVENT_TABLE(DebugFrame, wxFrame)
    EVT_TIMER(wxID_ANY, DebugFrame::OnTimer)
wxEND_EVENT_TABLE()

DebugFrame::DebugFrame(MainFrame* main, CDI& idc) :
    wxFrame(main, wxID_ANY, "Debug", wxDefaultPosition, wxSize(450, 500)),
    cdi(idc),
    mainFrame(main),
    refreshTimer(this),
    auiManager(this)
{
    wxPanel* panel = new wxPanel(this);
    logCPU = new wxCheckBox(panel, wxID_ANY, "CPU", wxDefaultPosition, wxSize(60, 15));
    logBIOS = new wxCheckBox(panel, wxID_ANY, "BIOS", wxDefaultPosition, wxSize(60, 15));
    logRAM = new wxCheckBox(panel, wxID_ANY, "RAM", wxDefaultPosition, wxSize(60, 15));
    logVDSC = new wxCheckBox(panel, wxID_ANY, "VDSC", wxDefaultPosition, wxSize(60, 15));
    logSlave = new wxCheckBox(panel, wxID_ANY, "Slave", wxDefaultPosition, wxSize(60, 15));
    logRTC = new wxCheckBox(panel, wxID_ANY, "RTC", wxDefaultPosition, wxSize(60, 15));

    memoryAccess = new GenericList(panel, [=] (wxListCtrl* frame) {
        wxListItem location;
        location.SetText("Location");
        location.SetWidth(60);
        frame->InsertColumn(0, location);

        wxListItem pc;
        pc.SetText("PC");
        pc.SetWidth(60);
        frame->InsertColumn(1, pc);

        wxListItem direction;
        direction.SetText("Direction");
        direction.SetWidth(60);
        frame->InsertColumn(2, direction);

        wxListItem type;
        type.SetText("Type");
        type.SetWidth(60);
        frame->InsertColumn(3, type);

        wxListItem address;
        address.SetText("Address");
        address.SetWidth(65);
        frame->InsertColumn(4, address);

        wxListItem value;
        value.SetText("Value");
        value.SetWidth(60);
        frame->InsertColumn(5, value);

        wxListItem valueHex;
        valueHex.SetText("Value Hex");
        valueHex.SetWidth(65);
        frame->InsertColumn(6, valueHex);
    }, [=] (long item, long column) -> std::string {
        std::lock_guard<std::mutex> lock(this->memoryAccessLogsMutex);
        if(item >= (long)memoryAccessLogs.size())
            return "";

        const LogMemoryAccess& log = this->memoryAccessLogs[item];
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

    wxSizer* logSizer = new wxBoxSizer(wxHORIZONTAL);
    logSizer->Add(logCPU);
    logSizer->Add(logBIOS);
    logSizer->Add(logRAM);
    logSizer->Add(logVDSC);
    logSizer->Add(logSlave);
    logSizer->Add(logRTC);

    wxSizer* verSizer = new wxBoxSizer(wxVERTICAL);
    verSizer->Add(logSizer);
    verSizer->Add(memoryAccess, wxSizerFlags().Expand().Proportion(1));

    panel->SetSizer(verSizer);

    auiManager.AddPane(panel, wxAuiPaneInfo().Caption("Memory Access").Center().CloseButton(false).Resizable());
    auiManager.Update();

    cdi.callbacks.SetOnLogMemoryAccess([=] (const LogMemoryAccess& arg) {
        if((arg.location == CPU   && this->logCPU->GetValue())   ||
           (arg.location == BIOS  && this->logBIOS->GetValue())  ||
           (arg.location == RAM   && this->logRAM->GetValue())   ||
           (arg.location == VDSC  && this->logVDSC->GetValue())  ||
           (arg.location == Slave && this->logSlave->GetValue()) ||
           (arg.location == RTC   && this->logRTC->GetValue())
           )
        {
            std::unique_lock<std::mutex> lock(this->memoryAccessLogsMutex);
            this->memoryAccessLogs.push_back(arg);
            lock.unlock();
            mainFrame->app.logMemoryAccess << \
                '[' << std::setw(5) << std::setfill(' ') << memoryAccessLocationToString(arg.location) << "] (0x" << \
                std::hex << std::setw(6) << std::setfill('0') << arg.pc << ") " << \
                arg.direction << " " << \
                arg.type << " at 0x" << \
                arg.address << " : 0x" << \
                arg.data << " " << std::dec << arg.data << std::endl;
        }
    });

    refreshTimer.Start(200);
}

DebugFrame::~DebugFrame()
{
    cdi.callbacks.SetOnLogMemoryAccess(nullptr);
    auiManager.UnInit();
    mainFrame->debugFrame = nullptr;
}

void DebugFrame::OnTimer(wxTimerEvent&)
{
    std::unique_lock<std::mutex> lock(this->memoryAccessLogsMutex);
    memoryAccess->SetItemCount(memoryAccessLogs.size());
    lock.unlock();
    memoryAccess->Update();
}
