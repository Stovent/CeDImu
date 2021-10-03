#ifndef GUI_DEBUGFRAME_HPP
#define GUI_DEBUGFRAME_HPP

#include "../CeDImu.hpp"
class MainFrame;
#include "GenericList.hpp"

#include <wx/aui/framemanager.h>
#include <wx/checkbox.h>
#include <wx/frame.h>
#include <wx/timer.h>

#include <mutex>

class DebugFrame : public wxFrame
{
public:
    CeDImu& m_cedimu;
    MainFrame* m_mainFrame;
    wxAuiManager m_auiManager;
    wxTimer m_updateTimer;

    wxCheckBox* m_logCpu;
    wxCheckBox* m_logBios;
    wxCheckBox* m_logRam;
    wxCheckBox* m_logVdsc;
    wxCheckBox* m_logSlave;
    wxCheckBox* m_logNvram;
    GenericList* m_memoryLogsList;
    bool m_updateMemoryLogs;
    std::mutex m_memoryLogsMutex;
    std::vector<LogMemoryAccess> m_memoryLogs;

    DebugFrame() = delete;
    DebugFrame(MainFrame* mainFrame, CeDImu& cedimu);
    ~DebugFrame();

    void UpdateManager(wxTimerEvent&);
    void UpdateMemoryLogs();

    wxDECLARE_EVENT_TABLE();
};

#endif // GUI_DEBUGFRAME_HPP
