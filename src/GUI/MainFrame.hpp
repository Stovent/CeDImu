#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

class MainFrame;

#include <wx/frame.h>

#include "../CeDImu.hpp"
#include "GamePanel.hpp"

enum
{
    IDOnOpenROM = wxID_LAST + 1,
    IDOnCloseROM,
    IDOnAbout,
};

class MainFrame : public wxFrame
{
public:
    MainFrame(CeDImu* appp, const wxString& title, const wxPoint& pos, const wxSize& size);
    void CreateMenuBar();

private:
    CeDImu* app;
    GamePanel* gamePanel;

    void OnAbout(wxCommandEvent& event);
    void OnCloseROM(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnOpenROM(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // MAINFRAME_HPP
