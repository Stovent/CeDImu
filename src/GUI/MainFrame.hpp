#ifndef GUI_MAINFRAME_HPP
#define GUI_MAINFRAME_HPP

#include <wx/frame.h>
#include <wx/timer.h>

class CeDImu;
class GamePanel;
class SettingsFrame;

class MainFrame : public wxFrame
{
public:
    CeDImu& m_cedimu;
    wxTimer m_updateTimer;
    wxMenuItem* m_pauseMenuItem;
    GamePanel* m_gamePanel;
    SettingsFrame* m_settingsFrame;

    uint64_t m_oldCycleCount;
    uint32_t m_oldFrameCount;

    MainFrame() = delete;
    MainFrame(CeDImu& cedimu);

    void CreateMenuBar();
    void UpdateTitle();
    void UpdateStatusBar();
    void UpdateUI(wxTimerEvent&);

    void OnOpenDisc(wxCommandEvent&);
    void OnCloseDisc(wxCommandEvent&);
    void OnScreenshot(wxCommandEvent&);
    void OnExit(wxCommandEvent&);
    void OnClose(wxCloseEvent&);

    void OnPause(wxCommandEvent&);
    void OnIncreaseSpeed(wxCommandEvent&);
    void OnDecreaseSpeed(wxCommandEvent&);
    void OnReloadCore(wxCommandEvent&);

    void OnExportAudio(wxCommandEvent&);
    void OnExportFiles(wxCommandEvent&);
    void OnExportVideo(wxCommandEvent&);
    void OnExportRawVideo(wxCommandEvent&);

    void OnSettings(wxCommandEvent&);
    void OnAbout(wxCommandEvent&);

    wxDECLARE_EVENT_TABLE();
};

#endif // GUI_MAINFRAME_HPP
