#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

class MainFrame;

#include "GamePanel.hpp"
class CeDImu;
#include "DisassemblerFrame.hpp"
#include "RAMSearchFrame.hpp"
class VDSCViewer;
class SlaveViewer;

#include <wx/frame.h>
#include <wx/menuitem.h>
#include <wx/timer.h>

class MainFrame : public wxFrame
{
public:
    CeDImu* app;
    GamePanel* gamePanel;
    wxMenuItem* pauseItem;

    MainFrame(CeDImu* appp, const wxString& title, const wxPoint& pos, const wxSize& size);
    DisassemblerFrame* disassemblerFrame;
    RAMSearchFrame* ramSearchFrame;
    VDSCViewer* vdscViewer;
    SlaveViewer* slaveViewer;

    void Pause();
    void RefreshTitle(wxTimerEvent& event);

private:
    wxTimer renderTimer;
    uint32_t oldFrameCount;
    uint64_t oldCycleCount;

    void CreateMenuBar();
    void OnOpenROM(wxCommandEvent& event);
    void OnOpenBinary(wxCommandEvent& event);
    void OnLoadBIOS(wxCommandEvent& event);
    void OnCloseROM(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);

    void OnPause(wxCommandEvent& event);
    void OnExecuteXInstructions(wxCommandEvent& event);
    void OnRebootCore(wxCommandEvent& event);

    void OnSlaveViewer(wxCommandEvent& event);
    void OnVDSCViewer(wxCommandEvent& event);
    void OnDisassembler(wxCommandEvent& event);
    void OnRAMSearch(wxCommandEvent& event);

    void OnExportFiles(wxCommandEvent& event);
    void OnExportAudio(wxCommandEvent& event);
    void OnExportVideo(wxCommandEvent& event);

    void OnSettings(wxCommandEvent& event);

    void OnAbout(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // MAINFRAME_HPP
