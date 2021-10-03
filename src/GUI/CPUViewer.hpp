#ifndef GUI_CPUVIEWER_HPP
#define GUI_CPUVIEWER_HPP

#include "../CeDImu.hpp"
class MainFrame;
#include "GenericList.hpp"

#include <wx/aui/framemanager.h>
#include <wx/frame.h>
#include <wx/listctrl.h>
#include <wx/textctrl.h>
#include <wx/timer.h>

#include <mutex>

class CPUViewer : public wxFrame
{
public:
    CeDImu& m_cedimu;
    MainFrame* m_mainFrame;
    wxAuiManager m_auiManager;
    wxTimer m_updateTimer;
    bool m_updateManager;

    wxListCtrl* m_internalList;

    GenericList* m_disassemblerList;
    std::mutex m_instructionsMutex;
    std::vector<Instruction> m_instructions;
    bool m_flushInstructions;

    wxTextCtrl* m_registers[20];

    uint8_t m_lastByte;
    wxTextCtrl* m_uartTextCtrl;

    CPUViewer() = delete;
    CPUViewer(MainFrame* mainFrame, CeDImu& cedimu);
    ~CPUViewer();

    void UpdateManager(wxTimerEvent&);
    void UpdateInternal();
    void UpdateRegisters();

    wxDECLARE_EVENT_TABLE();
};

#endif // GUI_CPUVIEWER_HPP
