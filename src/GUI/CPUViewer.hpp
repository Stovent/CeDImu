#ifndef GUI_CPUVIEWER_HPP
#define GUI_CPUVIEWER_HPP

#include "../CeDImu.hpp"
class MainFrame;
#include "GenericList.hpp"

#include <wx/aui/framemanager.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/frame.h>
#include <wx/listctrl.h>
#include <wx/textctrl.h>
#include <wx/timer.h>

#include <array>
#include <mutex>

class CPUViewer : public wxFrame
{
public:
    CeDImu& m_cedimu;
    MainFrame* m_mainFrame;
    wxAuiManager m_auiManager;
    wxTimer m_updateTimer;
    bool m_updateManager;

    // Internal registers
    wxListCtrl* m_internalList;

    // Disassembler
    GenericList* m_disassemblerList;
    std::mutex m_instructionsMutex;
    std::vector<LogInstruction> m_instructions;
    bool m_flushInstructions;

    // CPU registers
    std::array<wxTextCtrl*, 20> m_registersValue;
    std::array<wxChoice*, 8> m_registersSize; // Only data regs.
    std::array<wxCheckBox*, 8> m_registersHex; // Only data regs.
    wxTextCtrl* m_srDisassembled; // Letters instead of bits for clarity.

    // UART
    std::mutex m_uartMutex;
    std::vector<char> m_uartMissing;
    uint8_t m_lastByte;
    wxTextCtrl* m_uartTextCtrl;

    // Controls
    wxTextCtrl* m_executeCtrl;
    wxCheckBox* m_autoscrollDisassembler;

    CPUViewer() = delete;
    CPUViewer(MainFrame* mainFrame, CeDImu& cedimu);
    ~CPUViewer();

    void UpdateManager(wxTimerEvent&);
    void UpdateInternal();
    void UpdateCPURegisters();
    void UpdateUART();

    wxDECLARE_EVENT_TABLE();
};

#endif // GUI_CPUVIEWER_HPP
