#ifndef GUI_MAINFRAME_HPP
#define GUI_MAINFRAME_HPP

#include <wx/frame.h>

class CeDImu;
class SettingsFrame;

class MainFrame : public wxFrame
{
public:
    CeDImu& m_cedimu;
    SettingsFrame* m_settingsFrame;

    MainFrame() = delete;
    MainFrame(CeDImu& cedimu);

    void CreateMenuBar();

    void OnReloadCore(wxCommandEvent&);

    void OnClose(wxCloseEvent&);
    void OnExit(wxCommandEvent&);

    void OnSettings(wxCommandEvent&);
    void OnAbout(wxCommandEvent&);

    wxDECLARE_EVENT_TABLE();
};

#endif // GUI_MAINFRAME_HPP
