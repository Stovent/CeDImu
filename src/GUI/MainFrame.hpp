#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

class MainFrame;

#include "GamePanel.hpp"
class CeDImu;
#include "CPUViewer.hpp"
#include "RAMSearchFrame.hpp"
class VDSCViewer;

#include <wx/frame.h>
#include <wx/menuitem.h>
#include <wx/timer.h>

class MainFrame : public wxFrame
{
public:
    CeDImu& app;
    GamePanel* gamePanel;
    wxMenuItem* pauseItem;

    MainFrame(CeDImu& appp, const wxString& title, const wxPoint& pos, const wxSize& size);
    CPUViewer* cpuViewer;
    RAMSearchFrame* ramSearchFrame;
    VDSCViewer* vdscViewer;

    void Pause();
    void RefreshTitle();
    void RefreshStatusBar(wxTimerEvent& event);

private:
    wxTimer renderTimer;
    uint32_t oldFrameCount;
    uint64_t oldCycleCount;

    void CreateMenuBar();
    void OnOpenROM(wxCommandEvent& event);
    void OnCloseROM(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);

    void OnPause(wxCommandEvent& event);
    void OnExecuteXInstructions(wxCommandEvent& event);
    void OnReset(wxCommandEvent& event);
    void OnRebootCore(wxCommandEvent& event);
    void OnResizeView(wxCommandEvent& event);

    void OnExportFiles(wxCommandEvent& event);
    void OnExportAudio(wxCommandEvent& event);
    void OnExportVideo(wxCommandEvent& event);

    void OnCPUViewer(wxCommandEvent& event);
    void OnVDSCViewer(wxCommandEvent& event);
    void OnRAMSearch(wxCommandEvent& event);

    void OnSettings(wxCommandEvent& event);

    void OnAbout(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // MAINFRAME_HPP
