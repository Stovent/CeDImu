#include "MainFrame.hpp"
#include "enums.hpp"
#include "GamePanel.hpp"
#include "SettingsFrame.hpp"
#include "../CeDImu.hpp"

#include <wx/menu.h>
#include <wx/menuitem.h>
#include <wx/msgdlg.h>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_CLOSE(MainFrame::OnClose)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
    EVT_MENU(IDMainFrameOnPause, MainFrame::OnPause)
    EVT_MENU(IDMainFrameOnIncreaseSpeed, MainFrame::OnIncreaseSpeed)
    EVT_MENU(IDMainFrameOnDecreaseSpeed, MainFrame::OnDecreaseSpeed)
    EVT_MENU(IDMainFrameOnReloadCore, MainFrame::OnReloadCore)
    EVT_MENU(IDMainFrameOnSettings, MainFrame::OnSettings)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(CeDImu& cedimu) :
    wxFrame(NULL, wxID_ANY, "CeDImu", wxDefaultPosition, wxSize(400, 300)),
    m_cedimu(cedimu),
    m_settingsFrame(nullptr)
{
    CreateMenuBar();
    CreateStatusBar(2);
    SetStatusText(std::to_string(int(CPU_SPEEDS[m_cedimu.m_cpuSpeed] * 100)) + "%", 1);
    new GamePanel(this, m_cedimu);

    Show();
}

void MainFrame::CreateMenuBar()
{
    wxMenuBar* menuBar = new wxMenuBar();

    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(wxID_EXIT, "Close", "Closes CeDImu");
    menuBar->Append(fileMenu, "File");

    wxMenu* emulationMenu = new wxMenu();
    m_pauseMenuItem = emulationMenu->AppendCheckItem(IDMainFrameOnPause, "Pause\tPause", "Pause or resume emulation");
    m_pauseMenuItem->Check();
    emulationMenu->Append(IDMainFrameOnIncreaseSpeed, "Increase Speed", "Increase the emulation speed");
    emulationMenu->Append(IDMainFrameOnDecreaseSpeed, "Decrease Speed", "Decrease the emulation speed");
    emulationMenu->Append(IDMainFrameOnReloadCore, "Reload core\tCtrl+R");
    menuBar->Append(emulationMenu, "Emulation");

    wxMenu* optionsMenu = new wxMenu();
    optionsMenu->Append(IDMainFrameOnSettings, "Settings", "Edit CeDImu's settings");
    optionsMenu->Append(wxID_ABOUT, "About");
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

void MainFrame::OnPause(wxCommandEvent&)
{
    if(m_pauseMenuItem->IsChecked())
    {
        m_cedimu.StopEmulation();
    }
    else
    {
        m_cedimu.StartEmulation();
    }
}

void MainFrame::OnIncreaseSpeed(wxCommandEvent&)
{
    m_cedimu.IncreaseEmulationSpeed();
    SetStatusText(std::to_string(int(CPU_SPEEDS[m_cedimu.m_cpuSpeed] * 100)) + "%", 1);
}

void MainFrame::OnDecreaseSpeed(wxCommandEvent&)
{
    m_cedimu.DecreaseEmulationSpeed();
    SetStatusText(std::to_string(int(CPU_SPEEDS[m_cedimu.m_cpuSpeed] * 100)) + "%", 1);
}

void MainFrame::OnReloadCore(wxCommandEvent&)
{
    if(m_cedimu.InitCDI())
    {
        if(!m_pauseMenuItem->IsChecked())
            m_cedimu.StartEmulation();
        SetStatusText("Core reloaded");
    }
    else
        SetStatusText("Failed to reload core");
}

void MainFrame::OnSettings(wxCommandEvent&)
{
    if(m_settingsFrame == nullptr)
        m_settingsFrame = new SettingsFrame(this);
}

void MainFrame::OnAbout(wxCommandEvent&)
{
    wxMessageBox("CeDImu is a Philips CD-i player emulator, written by Stovent.");
}
