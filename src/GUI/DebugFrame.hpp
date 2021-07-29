#ifndef GUI_DEBUGFRAME_HPP
#define GUI_DEBUGFRAME_HPP

#include "../CDI/CDI.hpp"
class MainFrame;
#include "GenericList.hpp"

#include <wx/aui/aui.h>
#include <wx/checkbox.h>
#include <wx/frame.h>
#include <wx/timer.h>

#include <mutex>
#include <vector>

class DebugFrame : public wxFrame
{
public:
    CDI& cdi;
    MainFrame* mainFrame;
    std::mutex memoryAccessLogsMutex;
    std::vector<LogMemoryAccess> memoryAccessLogs;

    wxTimer refreshTimer;
    wxAuiManager auiManager;
    wxCheckBox* logCPU;
    wxCheckBox* logBIOS;
    wxCheckBox* logRAM;
    wxCheckBox* logVDSC;
    wxCheckBox* logSlave;
    wxCheckBox* logRTC;
    GenericList* memoryAccess;

    DebugFrame(MainFrame* main, CDI& idc);
    ~DebugFrame();

    void OnTimer(wxTimerEvent&);

    wxDECLARE_EVENT_TABLE();
};

#endif // GUI_DEBUGFRAME_HPP
