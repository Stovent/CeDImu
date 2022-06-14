#ifndef GUI_MAINFRAME_HPP
#define GUI_MAINFRAME_HPP

class CeDImu;
class CPUViewer;
class DebugFrame;
class GamePanel;
class SettingsFrame;
class VDSCViewer;

#include <wx/frame.h>
#include <wx/menuitem.h>
#include <wx/timer.h>

class MainFrame : public wxFrame
{
public:
    CeDImu& m_cedimu;
    wxTimer m_updateTimer;
    wxMenu* m_fileMenu;
    wxMenuItem* m_startBiosMenuItem;
    wxMenuItem* m_reloadCoreMenuItem;
    wxMenuItem* m_pauseMenuItem;

    GamePanel* m_gamePanel;
    CPUViewer* m_cpuViewer;
    SettingsFrame* m_settingsFrame;
    VDSCViewer* m_vdscViewer;
    DebugFrame* m_debugFrame;

    uint64_t m_oldCycleCount;
    uint32_t m_oldFrameCount;
    int m_biosIndex;

    MainFrame() = delete;
    explicit MainFrame(CeDImu& cedimu);

    void CreateMenuBar();
    void CreateBiosMenu();
    void UpdateTitle();
    void UpdateStatusBar();
    void UpdateUI(wxTimerEvent&);

    void OnStartBios(wxCommandEvent& event);
    void OnOpenDisc(wxCommandEvent&);
    void OnCloseDisc(wxCommandEvent&);
    void OnScreenshot(wxCommandEvent&);
    void OnExit(wxCommandEvent&);
    void OnClose(wxCloseEvent&);

    void OnPause(wxCommandEvent&);
    void OnSingleStep(wxCommandEvent&);
    void OnFrameAdvance(wxCommandEvent&);
    void OnIncreaseSpeed(wxCommandEvent&);
    void OnDecreaseSpeed(wxCommandEvent&);
    void OnReloadCore(wxCommandEvent&);
    void OnResizeView(wxCommandEvent&);

    void OnExportAudio(wxCommandEvent&);
    void OnExportFiles(wxCommandEvent&);
    void OnExportVideo(wxCommandEvent&);
    void OnExportRawVideo(wxCommandEvent&);

    void OnCPUViewer(wxCommandEvent&);
    void OnVDSCViewer(wxCommandEvent&);
    void OnDebugFrame(wxCommandEvent&);

    void OnSettings(wxCommandEvent&);
    void OnAbout(wxCommandEvent&);

    wxDECLARE_EVENT_TABLE();
};

#endif // GUI_MAINFRAME_HPP
