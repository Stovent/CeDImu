#include "MainFrame.hpp"
#include "enums.hpp"
#include "VDSCViewer.hpp"
#include "../Config.hpp"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/notebook.h>
#include <wx/sizer.h>

#include <cstdio>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(IDMainFrameOnOpenROM, MainFrame::OnOpenROM)
    EVT_MENU(IDMainFrameOnCloseROM, MainFrame::OnCloseROM)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
    EVT_MENU(IDMainFrameOnPause, MainFrame::OnPause)
    EVT_MENU(IDMainFrameOnExecuteXInstructions, MainFrame::OnExecuteXInstructions)
    EVT_MENU(IDMainFrameOnRebootCore,  MainFrame::OnRebootCore)
    EVT_MENU(IDMainFrameOnExportFiles, MainFrame::OnExportFiles)
    EVT_MENU(IDMainFrameOnExportAudio, MainFrame::OnExportAudio)
    EVT_MENU(IDMainFrameOnExportVideo, MainFrame::OnExportVideo)
    EVT_MENU(IDMainFrameOnCPUViewer,   MainFrame::OnCPUViewer)
    EVT_MENU(IDMainFrameOnVDSCViewer,  MainFrame::OnVDSCViewer)
    EVT_MENU(IDMainFrameOnRAMSearch,   MainFrame::OnRAMSearch)
    EVT_MENU(IDMainFrameOnSettings, MainFrame::OnSettings)
    EVT_MENU(IDMainFrameOnAbout, MainFrame::OnAbout)
    EVT_TIMER(wxID_ANY, MainFrame::RefreshTitle)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(CeDImu* appp, const wxString& title, const wxPoint& pos, const wxSize& size) : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    app = appp;
    gamePanel = new GamePanel(this, appp);
    cpuViewer = nullptr;
    ramSearchFrame = nullptr;
    vdscViewer = nullptr;
    oldFrameCount = 0;
    oldCycleCount = 0;

    CreateMenuBar();

    CreateStatusBar(4);
    SetStatusText("CeDImu");

    renderTimer.SetOwner(this);
    renderTimer.Start(1000);
}

void MainFrame::CreateMenuBar()
{
    wxMenu* file = new wxMenu;
    file->Append(IDMainFrameOnOpenROM, "Open ROM\tCtrl+O", "Choose the ROM to load");
    file->AppendSeparator();
    file->Append(IDMainFrameOnCloseROM, "Close ROM\tCtrl+Maj+C", "Close the ROM currently playing");
    file->Append(wxID_EXIT);

    wxMenu* emulation = new wxMenu;
    pauseItem = emulation->AppendCheckItem(IDMainFrameOnPause, "Pause");
    pauseItem->Check();
    emulation->Append(IDMainFrameOnExecuteXInstructions, "Execute X instructions\tCtrl+X");
    emulation->AppendSeparator();
    emulation->Append(IDMainFrameOnRebootCore, "Reboot Core\tCtrl+Maj+R");

    wxMenu* cdi = new wxMenu;
    wxMenu* cdiexport = new wxMenu;
    cdiexport->Append(IDMainFrameOnExportFiles, "Files");
    cdiexport->Append(IDMainFrameOnExportAudio, "Audio");
    cdiexport->Append(IDMainFrameOnExportVideo, "Video");
    cdi->AppendSubMenu(cdiexport, "Export");

    wxMenu* tools = new wxMenu;
    tools->Append(IDMainFrameOnCPUViewer, "CPU Viewer\tCtrl+C");
    tools->Append(IDMainFrameOnVDSCViewer, "VDSC Viewer\tCtrl+V");
    tools->Append(IDMainFrameOnRAMSearch, "RAM Search\tCtrl+R");

    wxMenu* config = new wxMenu;
    config->Append(IDMainFrameOnSettings, "Settings");

    wxMenu* help = new wxMenu;
    help->Append(IDMainFrameOnAbout, "About");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(file, "File");
    menuBar->Append(emulation, "Emulation");
    menuBar->Append(cdi, "CD-I");
    menuBar->Append(tools, "Tools");
    menuBar->Append(config, "Config");
    menuBar->Append(help, "Help");

    SetMenuBar(menuBar);
}

void MainFrame::RefreshTitle(wxTimerEvent& event)
{
    uint16_t fps = 0;
    if(app->cdi.board)
    {
        fps = app->cdi.board->GetTotalFrameCount() - oldFrameCount;
        oldFrameCount = app->cdi.board->GetTotalFrameCount();
    }

    double freq = 0.0;
    if(app->cdi.board)
    {
        freq = (app->cdi.board->cpu.totalCycleCount - (double)oldCycleCount) / 1000000.0;
        oldCycleCount = app->cdi.board->cpu.totalCycleCount;
    }

    const DiscTime discTime = app->cdi.disc.GetTime();

    SetStatusText("FPS: " + std::to_string(fps), 1);
    SetStatusText(std::to_string(freq) + " MHz", 2);
    SetStatusText("Disc: " + std::to_string(discTime.minute) + ':' + std::to_string(discTime.second) + ':' + std::to_string(discTime.sector), 3);
    SetTitle((!app->cdi.disc.gameName.empty() ? app->cdi.disc.gameName + " | " : "") + (app->cdi.board ? app->biosName + " | CeDImu" : "CeDImu"));
}

void MainFrame::OnOpenROM(wxCommandEvent& event)
{
    if(app->cdi.board == nullptr)
    {
        wxMessageBox("The BIOS has not been loaded yet.");
    }

    wxFileDialog openFileDialog(this, "Open ROM", Config::ROMDirectory, "", "All files (*.*)|*.*|Binary files (*.bin)|*.bin|.CUE File (*.cue)|*.cue", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

#ifdef _WIN32
    Config::ROMDirectory = openFileDialog.GetPath().BeforeLast('\\');
#else
    Config::ROMDirectory = openFileDialog.GetPath().BeforeLast('/');
#endif
    if(!app->cdi.disc.Open(openFileDialog.GetPath().ToStdString()))
    {
        wxMessageBox("Could not open ROM!");
        return;
    }

    if(Config::skipBIOS)
    {
        CDIFile* module = app->cdi.disc.GetFile(app->cdi.disc.mainModule);
        uint32_t size;
        uint8_t* d = module->GetContent(size);
        if(d != nullptr && size)
        {
            uint32_t address = 0;
            app->cdi.board->PutDataInMemory(d, size, address);
            // Get the module execution offset based on the module header,
            // assuming the loaded module will always be a program
            address += app->cdi.board->GetLong(address + 0x30, NoFlags);
            uint8_t arr[4] = {(uint8_t)(address >> 24), (uint8_t)(address >> 16), (uint8_t)(address >> 8), (uint8_t)(address)};
            app->cdi.board->WriteToBIOSArea(arr, 4, 4);
        }
    }

    if(app->cdi.board)
        if(!pauseItem->IsChecked())
            app->StartGameThread();
}

void MainFrame::OnCloseROM(wxCommandEvent& event)
{
    app->StopGameThread();
    app->cdi.disc.Close();
}

void MainFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
    app->StopGameThread();
    Close(true);
}

void MainFrame::OnPause(wxCommandEvent& event)
{
    if(pauseItem->IsChecked())
    {
        app->StopGameThread();
        SetStatusText("Pause");
    }
    else
    {
        app->StartGameThread();
        SetStatusText("Running");
    }
}

void MainFrame::Pause()
{
    if(pauseItem->IsChecked())
    {
        app->StartGameThread();
        pauseItem->Check(false);
        SetStatusText("Running");
    }
    else
    {
        app->StopGameThread();
        pauseItem->Check(true);
        SetStatusText("Pause");
    }
}

void MainFrame::OnExecuteXInstructions(wxCommandEvent& event)
{
    if(!app->cdi.board)
        return;

    wxFrame* genericFrame = new wxFrame(this, wxID_ANY, "Execute instructions", GetPosition(), wxSize(200, 60));
    wxTextCtrl* input = new wxTextCtrl(genericFrame, wxID_ANY);
    wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* button = new wxButton(genericFrame, wxID_ANY, "Execute");

    button->Bind(wxEVT_BUTTON, [this, genericFrame, input] (wxEvent& event) {
        for(int i = 0; i < stoi(input->GetValue().ToStdString()); i++)
            this->app->cdi.board->cpu.Run(false);
    });

    sizer->Add(input, 1, wxEXPAND);
    sizer->Add(button, 0, wxALIGN_RIGHT);

    genericFrame->SetSizer(sizer);
    genericFrame->Show();
}

void MainFrame::OnRebootCore(wxCommandEvent& event)
{
    if(app->cdi.board)
        app->InitializeCores();
}

void MainFrame::OnExportFiles(wxCommandEvent& event)
{
    SetStatusText("Exporting files...");
    if(app->cdi.disc.IsOpen())
        app->cdi.disc.ExportFiles();
    else
        wxMessageBox("No ROM loaded, no file to export");
    SetStatusText("Files exported!");
}

void MainFrame::OnExportAudio(wxCommandEvent& event)
{
    SetStatusText("Exporting audio...");
    if(app->cdi.disc.IsOpen())
        app->cdi.disc.ExportAudio();
    else
        wxMessageBox("No ROM loaded, no file to export");
    SetStatusText("Audio exported!");
}

void MainFrame::OnExportVideo(wxCommandEvent& event)
{
    SetStatusText("Exporting video...");
    if(app->cdi.disc.IsOpen())
        app->cdi.disc.ExportVideo();
    else
        wxMessageBox("No ROM loaded, no file to export");
    SetStatusText("Video exported!");
}

void MainFrame::OnCPUViewer(wxCommandEvent& event)
{
    if(cpuViewer != nullptr || !app->cdi.board)
        return;
    cpuViewer = new CPUViewer(app->cdi.board->cpu, this, this->GetPosition() + wxPoint(this->GetSize().GetWidth(), 0), wxSize(700, 650));
    cpuViewer->Show();
}

void MainFrame::OnVDSCViewer(wxCommandEvent& event)
{
    if(vdscViewer != nullptr || !app->cdi.board)
        return;
    vdscViewer = new VDSCViewer(this, *app->cdi.board);
    vdscViewer->Show();
}

void MainFrame::OnRAMSearch(wxCommandEvent& event)
{
    if(ramSearchFrame != nullptr || !app->cdi.board)
        return;
    ramSearchFrame = new RAMSearchFrame(*app->cdi.board, this, this->GetPosition() + wxPoint(50, 50), wxSize(410, 600));
    ramSearchFrame->Show();
}

void MainFrame::OnSettings(wxCommandEvent& event)
{
    wxFrame*    settingsFrame = new wxFrame(this, wxID_ANY, "Settings", GetPosition() + wxPoint(30, 30), wxDefaultSize);
    wxPanel*    settingsPanel = new wxPanel(settingsFrame);
    wxNotebook* notebook      = new wxNotebook(settingsPanel, wxID_ANY);
#ifdef _WIN32
    char separator = '\\';
#else
    char separator = '/';
#endif // _WIN32
    // General
    wxPanel* generalPage = new wxPanel(notebook);
    wxSizer* generalSizer = new wxBoxSizer(wxVERTICAL);
    wxSizer* systemSizer = new wxBoxSizer(wxHORIZONTAL);

    wxTextCtrl* systemText = new wxTextCtrl(generalPage, wxID_ANY, Config::systemBIOS);
    wxButton* selectSystem = new wxButton(generalPage, wxID_ANY, "Select system BIOS");
    selectSystem->Bind(wxEVT_BUTTON, [settingsFrame, systemText, separator] (wxEvent& event) {
               wxFileDialog openFileDialog(settingsFrame, "Load system BIOS", wxString(Config::systemBIOS).BeforeLast(separator), "", "All files (*.*)|*.*|Binary files (*.bin,*.rom)|*.bin,*.rom", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
               if(openFileDialog.ShowModal() == wxID_CANCEL)
                   return;

                systemText->SetValue(openFileDialog.GetPath());
    });
    systemSizer->Add(systemText, 1, wxALIGN_RIGHT, 5);
    systemSizer->Add(selectSystem, 1, wxALIGN_RIGHT, 5);

    generalSizer->Add(systemSizer);
    generalPage->SetSizer(generalSizer);
    notebook->AddPage(generalPage, "General");

    // Emulation
    wxPanel* emulationPage = new wxPanel(notebook);
    wxBoxSizer* emulationSizer = new wxBoxSizer(wxVERTICAL);

    wxCheckBox* skipBIOS = new wxCheckBox(emulationPage, wxID_ANY, "skip BIOS");
    skipBIOS->SetValue(Config::skipBIOS);

    wxCheckBox* NVRAMUseCurrentTime = new wxCheckBox(emulationPage, wxID_ANY, "NVRAM use current time");
    NVRAMUseCurrentTime->SetValue(Config::NVRAMUseCurrentTime);

    wxCheckBox* PAL = new wxCheckBox(emulationPage, wxID_ANY, "PAL mode");
    PAL->SetValue(Config::PAL);

    emulationSizer->Add(skipBIOS);
    emulationSizer->Add(NVRAMUseCurrentTime);
    emulationSizer->Add(PAL);
    emulationPage->SetSizer(emulationSizer);
    notebook->AddPage(emulationPage, "Emulation");


    wxBoxSizer* saveCancelPanel = new wxBoxSizer(wxHORIZONTAL);
    wxButton* save = new wxButton(settingsPanel, wxID_ANY, "Save");
    save->Bind(wxEVT_BUTTON, [settingsFrame, systemText, skipBIOS, NVRAMUseCurrentTime, PAL] (wxEvent& event) {
        Config::skipBIOS = skipBIOS->GetValue();
        Config::PAL = PAL->GetValue();
        Config::NVRAMUseCurrentTime = NVRAMUseCurrentTime->GetValue();
        Config::systemBIOS = systemText->GetValue();
        Config::saveConfig();
        settingsFrame->Destroy();
    });
    saveCancelPanel->Add(save, 1, wxALIGN_RIGHT, 5);

    wxButton* cancel = new wxButton(settingsPanel, wxID_ANY, "Cancel");
    cancel->Bind(wxEVT_BUTTON, [settingsFrame] (wxEvent& event) {
        settingsFrame->Destroy();
    });
    saveCancelPanel->Add(cancel, 1, wxALIGN_RIGHT, 5);


    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    boxSizer->Add(notebook, 1, wxEXPAND);
    boxSizer->Add(saveCancelPanel, 0, wxALIGN_RIGHT);

    settingsPanel->SetSizer(boxSizer);
    settingsFrame->Show();
}

void MainFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("CeDImu is an open-source Philips CD-I emulator created by Stovent.", "About CeDImu", wxOK);
}
