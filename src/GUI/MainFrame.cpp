#include "MainFrame.hpp"

#include <cstdio>

#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/notebook.h>
#include <wx/checkbox.h>

#include "../Config.hpp"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(IDMainFrameOnOpenROM, MainFrame::OnOpenROM)
    EVT_MENU(IDMainFrameOnLoadBIOS, MainFrame::OnLoadBIOS)
    EVT_MENU(IDMainFrameOnCloseROM, MainFrame::OnCloseROM)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
    EVT_MENU(IDMainFrameOnPause, MainFrame::OnPause)
    EVT_MENU(IDMainFrameOnLimitFPS, MainFrame::OnLimitFPS)
    EVT_MENU(IDMainFrameOnExecuteXInstructions, MainFrame::OnExecuteXInstructions)
    EVT_MENU(IDMainFrameOnRebootCore, MainFrame::OnRebootCore)
    EVT_MENU(IDMainFrameOnVDSCViewer, MainFrame::OnVDSCViewer)
    EVT_MENU(IDMainFrameOnDisassembler, MainFrame::OnDisassembler)
    EVT_MENU(IDMainFrameOnExportFiles, MainFrame::OnExportFiles)
    EVT_MENU(IDMainFrameOnExportAudio, MainFrame::OnExportAudio)
    EVT_MENU(IDMainFrameOnExportVideo, MainFrame::OnExportVideo)
    EVT_MENU(IDMainFrameOnRAMSearch, MainFrame::OnRAMSearch)
    EVT_MENU(IDMainFrameOnSettings, MainFrame::OnSettings)
    EVT_MENU(IDMainFrameOnAbout, MainFrame::OnAbout)
    EVT_TIMER(wxID_ANY, MainFrame::RefreshTitle)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(CeDImu* appp, const wxString& title, const wxPoint& pos, const wxSize& size) : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    app = appp;
    gamePanel = new GamePanel(this, appp);
    disassemblerFrame = nullptr;
    ramSearchFrame = nullptr;
    vdscViewer = nullptr;
    oldFrameCount = 0;
    oldCycleCount = 0;

    CreateMenuBar();

    CreateStatusBar();
    SetStatusText("CeDImu");

    renderTimer.SetOwner(this);
    renderTimer.Start(1000);
}

void MainFrame::CreateMenuBar()
{
    wxMenu* file = new wxMenu;
    file->Append(IDMainFrameOnOpenROM, "Open ROM\tCtrl+O", "Choose the ROM to load");
    file->Append(IDMainFrameOnLoadBIOS, "Load BIOS\tCtrl+B", "Load a CD-I BIOS");
    file->AppendSeparator();
    file->Append(IDMainFrameOnCloseROM, "Close ROM\tCtrl+Maj+C", "Close the ROM currently playing");
    file->Append(wxID_EXIT);

    wxMenu* emulation = new wxMenu;
    pauseItem = emulation->AppendCheckItem(IDMainFrameOnPause, "Pause");
    limitFPSItem = emulation->AppendCheckItem(IDMainFrameOnLimitFPS, "Limit FPS");
    limitFPSItem->Check(Config::limitFPS);
    emulation->Append(IDMainFrameOnExecuteXInstructions, "Execute X instructions\tCtrl+X");
    emulation->AppendSeparator();
    emulation->Append(IDMainFrameOnRebootCore, "Reboot Core\tCtrl+R");

    wxMenu* cdi = new wxMenu;
    wxMenu* cdiexport = new wxMenu;
    cdiexport->Append(IDMainFrameOnExportFiles, "Files");
    cdiexport->Append(IDMainFrameOnExportAudio, "Audio");
    cdiexport->Append(IDMainFrameOnExportVideo, "Video");
    cdi->AppendSubMenu(cdiexport, "Export");

    wxMenu* tools = new wxMenu;
    tools->Append(IDMainFrameOnVDSCViewer, "VDSC Viewer\tCtrl+V");
    tools->Append(IDMainFrameOnDisassembler, "Disassembler\tCtrl+D");
    tools->Append(IDMainFrameOnRAMSearch, "RAM Search\tCtrl+S");

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
    if(app->vdsc)
    {
        fps = app->vdsc->totalFrameCount - oldFrameCount;
        oldFrameCount = app->vdsc->totalFrameCount;
    }
    long double freq = 0.0;
    if(app->cpu)
    {
        freq = (app->cpu->totalCycleCount - (long double)oldCycleCount) / 1000000.0;
        oldCycleCount = app->cpu->totalCycleCount;
    }
    SetTitle((app->cdi ? (!app->cdi->gameName.empty() ? app->cdi->gameName + " | " : "") : "") + (app->vdsc ? (app->vdsc->biosLoaded ? app->vdsc->biosFilename + " | " : "") : "") + "CeDImu | FPS: " + std::to_string(fps) + " | " + std::to_string(freq) + " MHz");
}

void MainFrame::OnOpenROM(wxCommandEvent& event)
{
    if(app->vdsc == nullptr || !app->vdsc->biosLoaded)
    {
        wxMessageBox("The BIOS has not been loaded yet, please choose one.");
        OnLoadBIOS(event);
    }

    wxFileDialog openFileDialog(this, "Open ROM", Config::ROMPath, "", "All files (*.*)|*.*|Binary files (*.bin)|*.bin|.CUE File (*.cue)|*.cue", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

#ifdef _WIN32
    Config::ROMPath = openFileDialog.GetPath().BeforeLast('\\');
#else
    Config::ROMPath = openFileDialog.GetPath().BeforeLast('/');
#endif
    if(!app->InitializeCDI(openFileDialog.GetPath().ToStdString().c_str()))
    {
        wxMessageBox("Could not open ROM!");
        return;
    }

    if(Config::skipBIOS)
    {
        CDIFile* module = app->cdi->GetFile(app->cdi->mainModule);
        uint32_t size, address = 0;
        char* d = module->GetFileContent(size);
        if(d != nullptr && size)
        {
            app->vdsc->PutDataInMemory(d, size, address);
            // Get the module execution offset based on the module header,
            // assuming the loaded module will always be a program
            app->cpu->SetRegister(CPURegisters::PC, address + app->vdsc->GetLong(address + 0x30));
        }
    }

    if(app->vdsc->biosLoaded)
        if(!pauseItem->IsChecked())
            app->StartGameThread();
}

void MainFrame::OnLoadBIOS(wxCommandEvent& event)
{
    wxFileDialog openFileDialog(this, "Load BIOS", Config::BIOSPath, "", "All files (*.*)|*.*|Binary files (*.bin,*.rom)|*.bin,*.rom", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

#ifdef _WIN32
    Config::BIOSPath = openFileDialog.GetPath().BeforeLast('\\');
#else
    Config::BIOSPath = openFileDialog.GetPath().BeforeLast('/');
#endif

    if(!app->InitializeCores(openFileDialog.GetPath().ToStdString().data()))
        wxMessageBox("Could not load BIOS");
}

void MainFrame::OnCloseROM(wxCommandEvent& event)
{
    app->StopGameThread();
    app->cdi->CloseROM();
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

void MainFrame::OnLimitFPS(wxCommandEvent& event)
{
    if(limitFPSItem->IsChecked())
    {
        Config::limitFPS = true;
    }
    else
    {
        Config::limitFPS = false;
    }
}

void MainFrame::OnExecuteXInstructions(wxCommandEvent& event)
{
    if(!app->cpu)
        return;

    wxFrame* genericFrame = new wxFrame(this, wxID_ANY, "Execute instructions", GetPosition(), wxSize(200, 60));
    wxTextCtrl* input = new wxTextCtrl(genericFrame, wxID_ANY);
    wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* button = new wxButton(genericFrame, wxID_ANY, "Execute");

    button->Bind(wxEVT_BUTTON, [this, genericFrame, input] (wxEvent& event) {
        for(int i = 0; i < stoi(input->GetValue().ToStdString()); i++)
            this->app->cpu->Run(false);
    });

    sizer->Add(input, 1, wxEXPAND);
    sizer->Add(button, 0, wxALIGN_RIGHT);

    genericFrame->SetSizer(sizer);
    genericFrame->Show();
}

void MainFrame::OnRebootCore(wxCommandEvent& event)
{
    if(app->cpu)
        app->cpu->Reset();
}

void MainFrame::OnVDSCViewer(wxCommandEvent& event)
{
    if(vdscViewer != nullptr || !app->vdsc)
        return;
    vdscViewer = new VDSCViewer(this, app->vdsc);
    vdscViewer->Show();
}

void MainFrame::OnDisassembler(wxCommandEvent& event)
{
    if(disassemblerFrame != nullptr || !app->cpu)
        return;
    disassemblerFrame = new DisassemblerFrame(app->cpu, this, this->GetPosition() + wxPoint(this->GetSize().GetWidth(), 0), wxSize(500, 460));
    disassemblerFrame->Show();
}

void MainFrame::OnRAMSearch(wxCommandEvent& event)
{
    if(ramSearchFrame != nullptr || !app->vdsc)
        return;
    ramSearchFrame = new RAMSearchFrame(app->vdsc, this, this->GetPosition() + wxPoint(50, 50), wxSize(410, 600));
    ramSearchFrame->Show();
}

void MainFrame::OnExportFiles(wxCommandEvent& event)
{
    SetStatusText("Exporting files...");
    if(app->cdi)
        app->cdi->ExportFiles();
    else
        wxMessageBox("No ROM loaded, no file to export");
    SetStatusText("Files exported!");
}

void MainFrame::OnExportAudio(wxCommandEvent& event)
{
    SetStatusText("Exporting audio...");
    if(app->cdi)
        app->cdi->ExportAudio();
    SetStatusText("Audio exported!");
}

void MainFrame::OnExportVideo(wxCommandEvent& event)
{
    SetStatusText("Exporting video...");
    if(app->cdi)
        app->cdi->ExportVideo();
    SetStatusText("Video exported!");
}

void MainFrame::OnSettings(wxCommandEvent& event)
{
    wxFrame*    settingsFrame = new wxFrame(this, wxID_ANY, "Settings", GetPosition() + wxPoint(30, 30), wxDefaultSize);
    wxPanel*    settingsPanel = new wxPanel(settingsFrame);
    wxNotebook* notebook      = new wxNotebook(settingsPanel, wxID_ANY);


    wxPanel*    emulationPage = new wxPanel(notebook);

    wxCheckBox* skipBIOS = new wxCheckBox(emulationPage, wxID_ANY, "skip BIOS");
    if(Config::skipBIOS) skipBIOS->SetValue(true); else skipBIOS->SetValue(false);

    notebook->AddPage(emulationPage, "Emulation");


    wxBoxSizer* saveCancelPanel = new wxBoxSizer(wxHORIZONTAL);
    wxButton* save = new wxButton(settingsPanel, wxID_ANY, "Save");
    save->Bind(wxEVT_BUTTON, [settingsFrame, skipBIOS] (wxEvent& event) {
        if(skipBIOS->GetValue()) Config::skipBIOS = true; else Config::skipBIOS = false;
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
