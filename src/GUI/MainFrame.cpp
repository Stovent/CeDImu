#include "MainFrame.hpp"
#include "enums.hpp"
#include "CPUViewer.hpp"
#include "DebugFrame.hpp"
#include "GamePanel.hpp"
#include "SettingsFrame.hpp"
#include "VDSCViewer.hpp"
#include "../CeDImu.hpp"
#include "../Config.hpp"

#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_TIMER(wxID_ANY, MainFrame::UpdateUI)
    EVT_CLOSE(MainFrame::OnClose)
    EVT_MENU(IDMainFrameOnOpenDisc, MainFrame::OnOpenDisc)
    EVT_MENU(IDMainFrameOnCloseDisc, MainFrame::OnCloseDisc)
    EVT_MENU(IDMainFrameOnScreenshot, MainFrame::OnScreenshot)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
    EVT_MENU(IDMainFrameOnPause, MainFrame::OnPause)
    EVT_MENU(IDMainFrameOnSingleStep, MainFrame::OnSingleStep)
    EVT_MENU(IDMainFrameOnFrameAdvance, MainFrame::OnFrameAdvance)
    EVT_MENU(IDMainFrameOnIncreaseSpeed, MainFrame::OnIncreaseSpeed)
    EVT_MENU(IDMainFrameOnDecreaseSpeed, MainFrame::OnDecreaseSpeed)
    EVT_MENU(IDMainFrameOnReloadCore, MainFrame::OnReloadCore)
    EVT_MENU(IDMainFrameOnExportAudio, MainFrame::OnExportAudio)
    EVT_MENU(IDMainFrameOnExportFiles, MainFrame::OnExportFiles)
    EVT_MENU(IDMainFrameOnExportVideo, MainFrame::OnExportVideo)
    EVT_MENU(IDMainFrameOnExportRawVideo, MainFrame::OnExportRawVideo)
    EVT_MENU(IDMainFrameOnCPUViewer, MainFrame::OnCPUViewer)
    EVT_MENU(IDMainFrameOnVDSCViewer, MainFrame::OnVDSCViewer)
    EVT_MENU(IDMainFrameOnDebugFrame, MainFrame::OnDebugFrame)
    EVT_MENU(IDMainFrameOnSettings, MainFrame::OnSettings)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(CeDImu& cedimu) :
    wxFrame(NULL, wxID_ANY, "CeDImu", wxDefaultPosition, wxSize(400, 300)),
    m_cedimu(cedimu),
    m_updateTimer(this),
    m_gamePanel(new GamePanel(this, m_cedimu)),
    m_cpuViewer(nullptr),
    m_settingsFrame(nullptr),
    m_vdscViewer(nullptr),
    m_oldCycleCount(0),
    m_oldFrameCount(0)
{
    CreateMenuBar();
    CreateStatusBar(3);

    Show();
    m_updateTimer.Start(1000);
}

void MainFrame::CreateMenuBar()
{
    wxMenuBar* menuBar = new wxMenuBar();

    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(IDMainFrameOnOpenDisc, "Open disc\tCtrl+O");
    fileMenu->Append(IDMainFrameOnCloseDisc, "Close disc\tCtrl+C");
    fileMenu->AppendSeparator();
    fileMenu->Append(IDMainFrameOnScreenshot, "Take screenshot\tCtrl+Shift+S", "Save to file the current frame");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "Close", "Closes CeDImu");
    menuBar->Append(fileMenu, "File");

    wxMenu* emulationMenu = new wxMenu();
    m_pauseMenuItem = emulationMenu->AppendCheckItem(IDMainFrameOnPause, "Pause\tPause", "Pause or resume emulation");
    m_pauseMenuItem->Check();
    emulationMenu->Append(IDMainFrameOnSingleStep, "Single step\tG", "Execute a single instruction");
    emulationMenu->Append(IDMainFrameOnFrameAdvance, "Frame advance\tF", "Start emulation until the end of the next frame");
    emulationMenu->Append(IDMainFrameOnIncreaseSpeed, "Increase Speed", "Increase the emulation speed");
    emulationMenu->Append(IDMainFrameOnDecreaseSpeed, "Decrease Speed", "Decrease the emulation speed");
    emulationMenu->Append(IDMainFrameOnReloadCore, "Reload core\tCtrl+R");
    menuBar->Append(emulationMenu, "Emulation");

    wxMenu* cdiMenu = new wxMenu();
    cdiMenu->Append(IDMainFrameOnExportAudio, "Export audio");
    cdiMenu->Append(IDMainFrameOnExportFiles, "Export files");
    cdiMenu->Append(IDMainFrameOnExportVideo, "Export video");
    cdiMenu->Append(IDMainFrameOnExportRawVideo, "Export raw video");
    menuBar->Append(cdiMenu, "CD-I");

    wxMenu* toolsMenu = new wxMenu();
    toolsMenu->Append(IDMainFrameOnCPUViewer, "CPU Viewer");
    toolsMenu->Append(IDMainFrameOnVDSCViewer, "VDSC Viewer");
    toolsMenu->Append(IDMainFrameOnDebugFrame, "Debug");
    menuBar->Append(toolsMenu, "Tools");

    wxMenu* optionsMenu = new wxMenu();
    optionsMenu->Append(IDMainFrameOnSettings, "Settings", "Edit CeDImu's settings");
    optionsMenu->Append(wxID_ABOUT, "About");
    menuBar->Append(optionsMenu, "Options");

    SetMenuBar(menuBar);
}

void MainFrame::UpdateTitle()
{
    const std::string disc = m_cedimu.m_cdi.disc.gameName.length() ? m_cedimu.m_cdi.disc.gameName + " | " : "";
    const std::string bios = m_cedimu.m_cdi.board ? m_cedimu.m_cdi.board->name + " (" + m_cedimu.m_biosName + ") | " : "";
    SetTitle(disc + bios + "CeDImu");
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

void MainFrame::OnOpenDisc(wxCommandEvent&)
{
    wxFileDialog fileDlg(this, wxFileSelectorPromptStr, Config::discDirectory, wxEmptyString, "Binary files (*.bin;*.iso)|*.bin;*.iso|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if(fileDlg.ShowModal() == wxID_OK)
        if(!m_cedimu.m_cdi.disc.Open(fileDlg.GetPath().ToStdString()))
            wxMessageBox("Failed to open disc");
}

void MainFrame::OnCloseDisc(wxCommandEvent&)
{
    m_cedimu.m_cdi.disc.Close();
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
        if(m_vdscViewer)
            m_vdscViewer->m_updateLists = true;
    }
    else
    {
        m_gamePanel->m_stopOnNextFrame = false;
        m_cedimu.StartEmulation();
    }
}

void MainFrame::OnSingleStep(wxCommandEvent&)
{
    if(m_pauseMenuItem->IsChecked())
    {
        std::lock_guard<std::mutex> lock(m_cedimu.m_cdiBoardMutex);
        if(m_cedimu.m_cdi.board)
            m_cedimu.m_cdi.board->cpu.Run(false);
    }
}

void MainFrame::OnFrameAdvance(wxCommandEvent&)
{
    std::lock_guard<std::mutex> lock(m_cedimu.m_cdiBoardMutex);
    if(m_cedimu.m_cdi.board)
    {
        m_gamePanel->m_stopOnNextFrame = true;
        if(!m_cedimu.m_cdi.board->cpu.IsRunning())
            m_cedimu.m_cdi.board->cpu.Run(true);
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

void MainFrame::OnExportAudio(wxCommandEvent&)
{
    if(m_cedimu.m_cdi.disc.IsOpen())
    {
        wxDirDialog dirDlg(this, wxDirSelectorPromptStr, wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        if(dirDlg.ShowModal() == wxID_OK)
            m_cedimu.m_cdi.disc.ExportAudio(dirDlg.GetPath().ToStdString());
    }
    else
    {
        wxMessageBox("No disc opened");
    }
}

void MainFrame::OnExportFiles(wxCommandEvent&)
{
    if(m_cedimu.m_cdi.disc.IsOpen())
    {
        wxDirDialog dirDlg(this, wxDirSelectorPromptStr, wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        if(dirDlg.ShowModal() == wxID_OK)
            m_cedimu.m_cdi.disc.ExportFiles(dirDlg.GetPath().ToStdString());
    }
    else
    {
        wxMessageBox("No disc opened");
    }
}

void MainFrame::OnExportVideo(wxCommandEvent&)
{
    if(m_cedimu.m_cdi.disc.IsOpen())
    {
        wxDirDialog dirDlg(this, wxDirSelectorPromptStr, wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        if(dirDlg.ShowModal() == wxID_OK)
            m_cedimu.m_cdi.disc.ExportVideo(dirDlg.GetPath().ToStdString());
    }
    else
    {
        wxMessageBox("No disc opened");
    }
}

void MainFrame::OnExportRawVideo(wxCommandEvent&)
{
    if(m_cedimu.m_cdi.disc.IsOpen())
    {
        wxDirDialog dirDlg(this, wxDirSelectorPromptStr, wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        if(dirDlg.ShowModal() == wxID_OK)
            m_cedimu.m_cdi.disc.ExportRawVideo(dirDlg.GetPath().ToStdString());
    }
    else
    {
        wxMessageBox("No disc opened");
    }
}

void MainFrame::OnCPUViewer(wxCommandEvent&)
{
    if(m_cpuViewer == nullptr)
        m_cpuViewer = new CPUViewer(this, m_cedimu);
}

void MainFrame::OnVDSCViewer(wxCommandEvent&)
{
    if(m_vdscViewer == nullptr)
        m_vdscViewer = new VDSCViewer(this, m_cedimu);
}

void MainFrame::OnDebugFrame(wxCommandEvent&)
{
    if(m_debugFrame == nullptr)
        m_debugFrame = new DebugFrame(this, m_cedimu);
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
