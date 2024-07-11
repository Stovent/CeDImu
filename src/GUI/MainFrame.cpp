#include "MainFrame.hpp"
#include "enums.hpp"
#include "CPUViewer.hpp"
#include "DebugFrame.hpp"
#include "GamePanel.hpp"
#include "OS9Viewer.hpp"
#include "SettingsFrame.hpp"
#include "VDSCViewer.hpp"
#include "../CeDImu.hpp"
#include "../Config.hpp"
#include "../export.hpp"

#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>

#include <filesystem>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_TIMER(wxID_ANY, MainFrame::OnUpdateUI)
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
    EVT_MENU(IDMainFrameOnResizeView, MainFrame::OnResizeView)
    EVT_MENU(IDMainFrameOnExportAudio, MainFrame::OnExportAudio)
    EVT_MENU(IDMainFrameOnExportFiles, MainFrame::OnExportFiles)
    EVT_MENU(IDMainFrameOnExportVideo, MainFrame::OnExportVideo)
    EVT_MENU(IDMainFrameOnExportRawVideo, MainFrame::OnExportRawVideo)
    EVT_MENU(IDMainFrameOnCPUViewer, MainFrame::OnCPUViewer)
    EVT_MENU(IDMainFrameOnOS9Viewer, MainFrame::OnOS9Viewer)
    EVT_MENU(IDMainFrameOnVDSCViewer, MainFrame::OnVDSCViewer)
    EVT_MENU(IDMainFrameOnDebugFrame, MainFrame::OnDebugFrame)
    EVT_MENU(IDMainFrameOnSettings, MainFrame::OnSettings)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(CeDImu& cedimu)
    : wxFrame(NULL, wxID_ANY, "CeDImu", wxDefaultPosition, wxSize(400, 300))
    , m_cedimu(cedimu)
    , m_updateTimer(this)
    , m_fileMenu(nullptr)
    , m_startBiosMenuItem(nullptr)
    , m_reloadCoreMenuItem(nullptr)
    , m_pauseMenuItem(nullptr)
    , m_gamePanel(new GamePanel(this, m_cedimu))
    , m_cpuViewer(nullptr)
    , m_settingsFrame(nullptr)
    , m_os9Viewer(nullptr)
    , m_vdscViewer(nullptr)
    , m_debugFrame(nullptr)
    , m_oldCycleCount(0)
    , m_oldFrameCount(0)
    , m_biosIndex(-1)
{
    CreateMenuBar();
    CreateStatusBar(3);

    Show();
    m_updateTimer.Start(1000);
}

void MainFrame::CreateMenuBar()
{
    wxMenuBar* menuBar = new wxMenuBar();

    m_fileMenu = new wxMenu();
    m_fileMenu->Append(IDMainFrameOnOpenDisc, "Open disc\tCtrl+O");
    m_fileMenu->Append(IDMainFrameOnCloseDisc, "Close disc\tCtrl+C");
    m_fileMenu->AppendSeparator();
    m_fileMenu->Append(IDMainFrameOnScreenshot, "Take screenshot\tCtrl+Shift+S", "Save to file the current frame");
    m_fileMenu->AppendSeparator();
    m_fileMenu->Append(wxID_EXIT, "Close", "Closes CeDImu");
    menuBar->Append(m_fileMenu, "File");

    wxMenu* emulationMenu = new wxMenu();
    m_pauseMenuItem = emulationMenu->AppendCheckItem(IDMainFrameOnPause, "Pause\tPause", "Pause or resume emulation");
    m_pauseMenuItem->Check();
    emulationMenu->Append(IDMainFrameOnSingleStep, "Single step\tG", "Execute a single instruction");
    emulationMenu->Append(IDMainFrameOnFrameAdvance, "Frame advance\tF", "Start emulation until the end of the next frame");
    emulationMenu->Append(IDMainFrameOnIncreaseSpeed, "Increase Speed\tKP_PageUp", "Increase the emulation speed");
    emulationMenu->Append(IDMainFrameOnDecreaseSpeed, "Decrease Speed\tKP_PageDown", "Decrease the emulation speed");
    m_reloadCoreMenuItem = emulationMenu->Append(IDMainFrameOnReloadCore, "Reload core\tCtrl+R");
    m_reloadCoreMenuItem->Enable(false);
    emulationMenu->Append(IDMainFrameOnResizeView, "Resize View");
    menuBar->Append(emulationMenu, "Emulation");

    wxMenu* cdiMenu = new wxMenu();
    cdiMenu->Append(IDMainFrameOnExportAudio, "Export audio");
    cdiMenu->Append(IDMainFrameOnExportFiles, "Export files");
    cdiMenu->Append(IDMainFrameOnExportVideo, "Export video");
    cdiMenu->Append(IDMainFrameOnExportRawVideo, "Export raw video");
    menuBar->Append(cdiMenu, "CD-I");

    wxMenu* toolsMenu = new wxMenu();
    toolsMenu->Append(IDMainFrameOnCPUViewer, "CPU Viewer");
    toolsMenu->Append(IDMainFrameOnOS9Viewer, "OS-9 Viewer");
    toolsMenu->Append(IDMainFrameOnVDSCViewer, "VDSC Viewer");
    toolsMenu->Append(IDMainFrameOnDebugFrame, "Debug");
    menuBar->Append(toolsMenu, "Tools");

    wxMenu* optionsMenu = new wxMenu();
    optionsMenu->Append(IDMainFrameOnSettings, "Settings", "Edit CeDImu's settings");
    optionsMenu->Append(wxID_ABOUT, "About");
    menuBar->Append(optionsMenu, "Options");

    CreateBiosMenu();
    SetMenuBar(menuBar);
}

void MainFrame::CreateBiosMenu()
{
    if(m_startBiosMenuItem != nullptr)
    {
        m_fileMenu->Destroy(m_startBiosMenuItem);
        m_startBiosMenuItem = nullptr;
    }

    wxMenu* startBiosMenu = new wxMenu();

    int i = -1;
    for(const Config::BiosConfig& biosEntry : Config::bioses)
    {
        startBiosMenu->Append(IDMainFrameBiosMenuBaseIndex + ++i, biosEntry.name);
    }

    if(i >= 0)
        startBiosMenu->Bind(wxEVT_MENU, &MainFrame::OnStartBios, this, IDMainFrameBiosMenuBaseIndex, IDMainFrameBiosMenuBaseIndex + i);
    m_startBiosMenuItem = m_fileMenu->Prepend(wxID_ANY, "Start BIOS", startBiosMenu);
    m_reloadCoreMenuItem->Enable(false); // In case the currently loaded BIOS is deleted from the settings.
}

void MainFrame::OnUpdateUI(wxTimerEvent&)
{
    UpdateUI();
}

void MainFrame::UpdateUI()
{
    std::string biosTitle;
    std::string discTitle;
    uint64_t cycleRate = 0;
    uint32_t frameRate = 0;

    {
        if(!m_cedimu.m_disc.m_gameName.empty())
            discTitle = m_cedimu.m_disc.m_gameName + " | ";

        std::lock_guard<std::recursive_mutex> lock(m_cedimu.m_cdiMutex);
        if(m_cedimu.m_cdi)
        {
            // Title.
            biosTitle = m_cedimu.m_cdi->m_boardName + " (" + m_cedimu.m_biosName + ") | ";
            if(!m_cedimu.m_cdi->m_disc.m_gameName.empty())
                discTitle = m_cedimu.m_cdi->m_disc.m_gameName + " | ";

            // Status bar.
            cycleRate = m_cedimu.m_cdi->m_cpu.totalCycleCount - m_oldCycleCount;
            m_oldCycleCount = m_cedimu.m_cdi->m_cpu.totalCycleCount;

            const uint32_t fc = m_cedimu.m_cdi->GetTotalFrameCount();
            frameRate = fc - m_oldFrameCount;
            m_oldFrameCount = fc;
        }
    }

    SetTitle(discTitle + biosTitle + "CeDImu");

    SetStatusText(std::to_string(int(CPU_SPEEDS[m_cedimu.m_cpuSpeed] * 100)) + "% (" + std::to_string(cycleRate) + " Hz)", 1);
    SetStatusText(std::to_string(m_oldFrameCount) + " / " + std::to_string(frameRate) + " FPS", 2);
}

void MainFrame::OnStartBios(wxCommandEvent& event)
{
    const int index = event.GetId() - IDMainFrameBiosMenuBaseIndex;
    const Config::BiosConfig& biosConfig = Config::bioses[index];

    if(m_cedimu.InitCDI(biosConfig))
    {
        m_gamePanel->Reset();
        if(!m_pauseMenuItem->IsChecked())
            m_cedimu.StartEmulation();
        m_biosIndex = index;
        m_reloadCoreMenuItem->Enable(true);
        SetStatusText("BIOS " + biosConfig.name + " loaded");
    }
    else
    {
        m_biosIndex = -1;
        m_reloadCoreMenuItem->Enable(false);
        SetStatusText("Failed to load " + biosConfig.name + " BIOS");
    }
}

void MainFrame::OnOpenDisc(wxCommandEvent&)
{
    wxFileDialog fileDlg(this, wxFileSelectorPromptStr, Config::discDirectory, wxEmptyString, "Binary files (*.bin;*.iso)|*.bin;*.iso|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if(fileDlg.ShowModal() == wxID_OK)
        if(!m_cedimu.OpenDisc(fileDlg.GetPath().ToStdString()))
            wxMessageBox("Failed to open disc");
}

void MainFrame::OnCloseDisc(wxCommandEvent&)
{
    m_cedimu.CloseDisc();
}

void MainFrame::OnScreenshot(wxCommandEvent&)
{
    std::lock_guard<std::recursive_mutex> lock(m_cedimu.m_cdiMutex);
    if(!m_cedimu.m_cdi)
        return;

    const bool isRunning = !m_pauseMenuItem->IsChecked();
    if(isRunning)
        m_cedimu.StopEmulation();

    uint32_t fc = m_cedimu.m_cdi->GetTotalFrameCount();
    wxFileDialog fileDlg(this, wxFileSelectorPromptStr, wxEmptyString, "frame_" + std::to_string(fc) + ".png", "PNG (*.png)|*.png", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if(fileDlg.ShowModal() == wxID_OK)
        if(!m_gamePanel->SaveScreenshot(fileDlg.GetPath().ToStdString()))
            wxMessageBox("Failed to save screenshot");

    if(isRunning)
        m_cedimu.StartEmulation();
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
        if(m_vdscViewer != nullptr)
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
        std::lock_guard<std::recursive_mutex> lock(m_cedimu.m_cdiMutex);
        if(m_cedimu.m_cdi)
            m_cedimu.m_cdi->m_cpu.Run(false);
    }
}

void MainFrame::OnFrameAdvance(wxCommandEvent&)
{
    std::lock_guard<std::recursive_mutex> lock(m_cedimu.m_cdiMutex);
    if(m_cedimu.m_cdi)
    {
        m_gamePanel->m_stopOnNextFrame = true;
        if(!m_cedimu.m_cdi->m_cpu.IsRunning())
            m_cedimu.m_cdi->m_cpu.Run(true);
    }
}

void MainFrame::OnIncreaseSpeed(wxCommandEvent&)
{
    m_cedimu.IncreaseEmulationSpeed();
    UpdateUI();
}

void MainFrame::OnDecreaseSpeed(wxCommandEvent&)
{
    m_cedimu.DecreaseEmulationSpeed();
    UpdateUI();
}

void MainFrame::OnReloadCore(wxCommandEvent&)
{
    const Config::BiosConfig& biosConfig = Config::bioses[m_biosIndex];

    if(m_cedimu.InitCDI(biosConfig))
    {
        m_gamePanel->Reset();
        if(!m_pauseMenuItem->IsChecked())
            m_cedimu.StartEmulation();
        SetStatusText("Core reloaded");
    }
    else
    {
        m_reloadCoreMenuItem->Enable(false);
        SetStatusText("Failed to reload core");
    }
}

void MainFrame::OnResizeView(wxCommandEvent&)
{
    const wxSize size = m_gamePanel->m_screen.GetSize();
    if(size.x > 0 && size.y > 0)
    {
        SetClientSize(size.x, size.y);
        Refresh();
    }
}

void MainFrame::OnExportAudio(wxCommandEvent&)
{
    CDIDisc& disc = m_cedimu.GetDisc();

    if(disc.IsOpen())
    {
        wxDirDialog dirDlg(this, wxDirSelectorPromptStr, wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        try
        {
            if(dirDlg.ShowModal() == wxID_OK)
                disc.ExportAudio(dirDlg.GetPath().ToStdString());
        }
        catch(const std::filesystem::filesystem_error& e)
        {
            std::string msg = "Failed to create directory \"" + e.path1().string() + "\".\nReason: ";
            wxMessageBox(msg + e.what());
        }
    }
    else
    {
        wxMessageBox("No disc opened");
    }
}

void MainFrame::OnExportFiles(wxCommandEvent&)
{
    CDIDisc& disc = m_cedimu.GetDisc();

    if(disc.IsOpen())
    {
        wxDirDialog dirDlg(this, wxDirSelectorPromptStr, wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        try
        {
            if(dirDlg.ShowModal() == wxID_OK)
                disc.ExportFiles(dirDlg.GetPath().ToStdString());
        }
        catch(const std::filesystem::filesystem_error& e)
        {
            std::string msg = "Failed to create directory \"" + e.path1().string() + "\".\nReason: ";
            wxMessageBox(msg + e.what());
        }
    }
    else
    {
        wxMessageBox("No disc opened");
    }
}

void MainFrame::OnExportVideo(wxCommandEvent&)
{
    CDIDisc& disc = m_cedimu.GetDisc();

    if(disc.IsOpen())
    {
        wxDirDialog dirDlg(this, wxDirSelectorPromptStr, wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        try
        {
            if(dirDlg.ShowModal() == wxID_OK)
                exportVideo(disc, dirDlg.GetPath().ToStdString());
        }
        catch(const std::filesystem::filesystem_error& e)
        {
            std::string msg = "Failed to create directory \"" + e.path1().string() + "\".\nReason: ";
            wxMessageBox(msg + e.what());
        }
    }
    else
    {
        wxMessageBox("No disc opened");
    }
}

void MainFrame::OnExportRawVideo(wxCommandEvent&)
{
    CDIDisc& disc = m_cedimu.GetDisc();

    if(disc.IsOpen())
    {
        wxDirDialog dirDlg(this, wxDirSelectorPromptStr, wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        try
        {
            if(dirDlg.ShowModal() == wxID_OK)
                disc.ExportRawVideo(dirDlg.GetPath().ToStdString());
        }
        catch(const std::filesystem::filesystem_error& e)
        {
            std::string msg = "Failed to create directory \"" + e.path1().string() + "\".\nReason: ";
            wxMessageBox(msg + e.what());
        }
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

void MainFrame::OnOS9Viewer(wxCommandEvent&)
{
    if(m_os9Viewer == nullptr && m_cedimu.m_cdi)
        m_os9Viewer = new OS9Viewer(this, m_cedimu);
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
    wxMessageBox("CeDImu is a Philips CD-i player emulator, written in C++ using wxWidgets.");
}
