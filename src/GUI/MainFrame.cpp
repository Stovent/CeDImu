#include "MainFrame.hpp"
#include "enums.hpp"
#include "GamePanel.hpp"
#include "SettingsFrame.hpp"
#include "../CeDImu.hpp"

#include <wx/dirdlg.h>
#include <wx/menu.h>
#include <wx/menuitem.h>
#include <wx/msgdlg.h>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_TIMER(wxID_ANY, MainFrame::UpdateUI)
    EVT_CLOSE(MainFrame::OnClose)
    EVT_MENU(IDMainFrameOnScreenshot, MainFrame::OnScreenshot)
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
    m_updateTimer(this),
    m_settingsFrame(nullptr),
    m_oldCycleCount(0),
    m_oldFrameCount(0)
{
    CreateMenuBar();
    CreateStatusBar(3);

    m_gamePanel = new GamePanel(this, m_cedimu);

    Show();
    m_updateTimer.Start(1000);
}

void MainFrame::CreateMenuBar()
{
    wxMenuBar* menuBar = new wxMenuBar();

    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(IDMainFrameOnScreenshot, "Take screenshot\tCtrl+Shift+S", "Save to file the current frame");
    fileMenu->AppendSeparator();
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

void MainFrame::UpdateTitle()
{
    const std::string bios = m_cedimu.m_biosName.size() ? m_cedimu.m_biosName + " | " : "";
    SetTitle(bios + "CeDImu");
}

void MainFrame::UpdateStatusBar()
{
    std::lock_guard<std::mutex> lock(m_cedimu.m_cdiBoardMutex);

    uint64_t cycleRate = 0;
    uint32_t frameRate = 0;
    if(m_cedimu.m_cdi.board)
    {
        cycleRate = m_cedimu.m_cdi.board->cpu.totalCycleCount - m_oldCycleCount;
        m_oldCycleCount = m_cedimu.m_cdi.board->cpu.totalCycleCount;

        const uint32_t fc = m_cedimu.m_cdi.board->GetTotalFrameCount();
        frameRate = fc - m_oldFrameCount;
        m_oldFrameCount = fc;
    }

    SetStatusText(std::to_string(int(CPU_SPEEDS[m_cedimu.m_cpuSpeed] * 100)) + "% (" + std::to_string(cycleRate) + " Hz)", 1);
    SetStatusText(std::to_string(m_oldFrameCount) + " / " + std::to_string(frameRate) + " FPS", 2);
}

void MainFrame::UpdateUI(wxTimerEvent&)
{
    UpdateTitle();
    UpdateStatusBar();
}

void MainFrame::OnScreenshot(wxCommandEvent&)
{
    wxDirDialog dirDlg(this, wxDirSelectorPromptStr, wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if(dirDlg.ShowModal() == wxID_OK)
        if(!m_gamePanel->SaveScreenshot(dirDlg.GetPath().ToStdString()))
            wxMessageBox("Failed to save screenshot");
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
    UpdateStatusBar();
}

void MainFrame::OnDecreaseSpeed(wxCommandEvent&)
{
    m_cedimu.DecreaseEmulationSpeed();
    UpdateStatusBar();
}

void MainFrame::OnReloadCore(wxCommandEvent&)
{
    if(m_cedimu.InitCDI())
    {
        m_gamePanel->Reset();
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
