#ifndef GUI_MAINFRAME_HPP
#define GUI_MAINFRAME_HPP

#include <wx/frame.h>
#include <wx/timer.h>

class CeDImu;
class SettingsFrame;

class MainFrame : public wxFrame
{
public:
    CeDImu& m_cedimu;
    wxTimer m_updateTimer;
    wxMenuItem* m_pauseMenuItem;
    SettingsFrame* m_settingsFrame;

    uint64_t m_oldCycleCount;
    uint32_t m_oldFrameCount;

    MainFrame() = delete;
    MainFrame(CeDImu& cedimu);

    void CreateMenuBar();
    void UpdateTitle();
    void UpdateStatusBar();
    void UpdateUI(wxTimerEvent&);

    void OnPause(wxCommandEvent&);
    void OnIncreaseSpeed(wxCommandEvent&);
    void OnDecreaseSpeed(wxCommandEvent&);
    void OnReloadCore(wxCommandEvent&);

    void OnClose(wxCloseEvent&);
    void OnExit(wxCommandEvent&);

    void OnSettings(wxCommandEvent&);
    void OnAbout(wxCommandEvent&);

    wxDECLARE_EVENT_TABLE();
};

#endif // GUI_MAINFRAME_HPP
