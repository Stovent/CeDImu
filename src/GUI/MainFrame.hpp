#ifndef GUI_MAINFRAME_HPP
#define GUI_MAINFRAME_HPP

#include <wx/frame.h>

class SettingsFrame;

class MainFrame : public wxFrame
{
public:
    SettingsFrame* m_settingsFrame;

    MainFrame();

    void CreateMenuBar();

    void OnClose(wxCloseEvent&);
    void OnExit(wxCommandEvent&);

    void OnSettings(wxCommandEvent&);

    wxDECLARE_EVENT_TABLE();
};

#endif // GUI_MAINFRAME_HPP
