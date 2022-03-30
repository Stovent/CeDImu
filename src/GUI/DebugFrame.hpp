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

    wxCheckBox* m_writeCpu;
    wxCheckBox* m_writeBios;
    wxCheckBox* m_writeRam;
    wxCheckBox* m_writeVdsc;
    wxCheckBox* m_writeSlave;
    wxCheckBox* m_writeCdic;
    wxCheckBox* m_writeNvram;
    wxCheckBox* m_writeOutOfRange;
    wxCheckBox* m_writeExceptions;

    wxCheckBox* m_logCpu;
    wxCheckBox* m_logBios;
    wxCheckBox* m_logRam;
    wxCheckBox* m_logVdsc;
    wxCheckBox* m_logSlave;
    wxCheckBox* m_logCdic;
    wxCheckBox* m_logNvram;
    wxCheckBox* m_logOutOfRange;
    GenericList* m_memoryLogsList;
    bool m_updateMemoryLogs;
    std::mutex m_memoryLogsMutex;
    std::vector<LogMemoryAccess> m_memoryLogs;

    GenericList* m_exceptionsList;
    std::mutex m_exceptionsMutex;
    std::vector<std::pair<size_t, LogSCC68070Exception>> m_exceptions; // <trap index starting at 1, exception>
    size_t m_trapCount;
    bool m_updateExceptions;

    DebugFrame() = delete;
    DebugFrame(MainFrame* mainFrame, CeDImu& cedimu);
    ~DebugFrame();

    void UpdateManager(wxTimerEvent&);

    wxDECLARE_EVENT_TABLE();
};

#endif // GUI_DEBUGFRAME_HPP
