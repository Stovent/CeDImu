#include "MainFrame.hpp"
#include "enums.hpp"
#include "SettingsFrame.hpp"

#include <wx/menu.h>
#include <wx/menuitem.h>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_CLOSE(MainFrame::OnClose)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
    EVT_MENU(IDMainFrameOnSettings, MainFrame::OnSettings)
wxEND_EVENT_TABLE()

MainFrame::MainFrame() :
    wxFrame(NULL, wxID_ANY, "CeDImu", wxDefaultPosition, wxSize(400, 300)),
    m_settingsFrame(nullptr)
{
    CreateMenuBar();

    Show();
}

void MainFrame::CreateMenuBar()
{
    wxMenuBar* menuBar = new wxMenuBar();

    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(wxID_EXIT, "Close", "Closes CeDImu");
    menuBar->Append(fileMenu, "File");

    wxMenu* optionsMenu = new wxMenu();
    optionsMenu->Append(IDMainFrameOnSettings, "Settings", "Edit CeDImu's settings");
    menuBar->Append(optionsMenu, "Options");

    SetMenuBar(menuBar);
}

void MainFrame::OnExit(wxCommandEvent&)
{
    Close();
}

void MainFrame::OnClose(wxCloseEvent&)
{
    Destroy();
}

void MainFrame::OnSettings(wxCommandEvent&)
{
    if(m_settingsFrame == nullptr)
        m_settingsFrame = new SettingsFrame(this);
}
