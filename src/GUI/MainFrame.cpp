#include "MainFrame.hpp"

#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>

#include <cstdio>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(IDOnOpenROM,   MainFrame::OnOpenROM)
    EVT_MENU(IDOnOpenBinary,   MainFrame::OnOpenBinary)
    EVT_MENU(IDOnLoadBIOS,   MainFrame::OnLoadBIOS)
    EVT_MENU(IDOnCloseROM,   MainFrame::OnCloseROM)
    EVT_MENU(wxID_EXIT,  MainFrame::OnExit)
    EVT_MENU(IDOnPause, MainFrame::OnPause)
    EVT_MENU(IDOnRebootCore, MainFrame::OnRebootCore)
    EVT_MENU(IDOnDisassembler, MainFrame::OnDisassembler)
    EVT_MENU(IDOnExportFiles, MainFrame::OnExportFiles)
    EVT_MENU(IDOnRAMWatch, MainFrame::OnRAMWatch)
    EVT_MENU(IDOnAbout, MainFrame::OnAbout)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(CeDImu* appp, const wxString& title, const wxPoint& pos, const wxSize& size) : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    app = appp;
    gamePanel = new GamePanel(this, appp);
    disassemblerFrame = nullptr;
    ramWatchFrame = nullptr;

    CreateMenuBar();

    CreateStatusBar();
    SetStatusText("CeDImu");
}

void MainFrame::CreateMenuBar()
{
    wxMenu* file = new wxMenu;
    file->Append(IDOnOpenROM, "Open ROM\tCtrl+O", "Choose the ROM to load");
    file->Append(IDOnOpenBinary, "Open Binary File\t", "Open m68k binary file");
    file->Append(IDOnLoadBIOS, "Load BIOS\t", "Load a CD-I BIOS");
    file->AppendSeparator();
    file->Append(IDOnCloseROM, "Close ROM\tCtrl+Maj+O", "Close the ROM currently playing");
    file->Append(wxID_EXIT);

    wxMenu* emulation = new wxMenu;
    pause = emulation->AppendCheckItem(IDOnPause, "Pause");
    emulation->AppendSeparator();
    emulation->Append(IDOnRebootCore, "Reboot Core\tCtrl+R");

    wxMenu* cdi = new wxMenu;
    wxMenu* cdiexport = new wxMenu;
    cdiexport->Append(IDOnExportFiles, "Files");
    cdi->AppendSubMenu(cdiexport, "Export");

    wxMenu* tools = new wxMenu;
    tools->Append(IDOnDisassembler, "Disassembler\tCtrl+D");
    tools->Append(IDOnRAMWatch, "RAM Watch\tCtrl+W");

    wxMenu* help = new wxMenu;
    help->Append(IDOnAbout, "About");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(file, "File");
    menuBar->Append(emulation, "Emulation");
    menuBar->Append(cdi, "CD-I");
    menuBar->Append(tools, "Tools");
    menuBar->Append(help, "Help");

    SetMenuBar(menuBar);
}

void MainFrame::OnOpenROM(wxCommandEvent& event)
{
    app->vdsc->ResetMemory();
    wxFileDialog openFileDialog(this, _("Open ROM"), "", "", "All files (*.*)|*.*|Binary files (*.bin)|*.bin|.CUE File (*.cue)|*.cue", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    if(!app->cdi->OpenROM(openFileDialog.GetFilename().ToStdString(), openFileDialog.GetDirectory().ToStdString() + "/"))
    {
        wxMessageBox("Could not open ROM!");
        return;
    }

    app->cpu->RebootCore();
    if(!pause->IsChecked())
        app->StartGameThread();
}

void MainFrame::OnOpenBinary(wxCommandEvent& event)
{
    app->vdsc->ResetMemory();
    wxFileDialog openFileDialog(this, _("Open ROM"), "", "", "Binary files (*.bin)|*.bin|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    FILE* f = fopen(openFileDialog.GetPath().ToStdString().data(), "rb");
    if(f == NULL)
        wxMessageBox("Could not open ROM!");

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    uint8_t* s = new uint8_t[size];
    fseek(f, 0, SEEK_SET);
    fread(s, 1, size, f);
    fclose(f);
    app->vdsc->PutDataInMemory(s, size, 0);
    app->cpu->RebootCore();

    if(!pause->IsChecked())
        app->StartGameThread();
}

void MainFrame::OnLoadBIOS(wxCommandEvent& event)
{
    wxFileDialog openFileDialog(this, _("Load BIOS"), "", "", "All files (*.*)|*.*|Binary files (*.bin,*.rom)|*.bin,*.rom", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    if(!app->vdsc->LoadBIOS(openFileDialog.GetPath().ToStdString().data()))
        wxMessageBox("Could not load BIOS");
}

void MainFrame::OnCloseROM(wxCommandEvent& event)
{
    app->StopGameThread();
}

void MainFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void MainFrame::OnRebootCore(wxCommandEvent& event)
{
    app->cpu->RebootCore();
}

void MainFrame::OnPause(wxCommandEvent& event)
{
    OnPause();
}

void MainFrame::OnPause()
{
    if(pause->IsChecked())
        app->StopGameThread();
    else
        app->StartGameThread();
}

void MainFrame::OnDisassembler(wxCommandEvent& event)
{
    if(disassemblerFrame == nullptr)
        disassemblerFrame = new DisassemblerFrame(*(app->cpu), this, this->GetPosition() + wxPoint(0, 50), wxSize(600, 400));
    disassemblerFrame->Show();
}

void MainFrame::OnRAMWatch(wxCommandEvent& event)
{
    if(ramWatchFrame == nullptr)
        ramWatchFrame = new RAMWatchFrame(app->vdsc, this, this->GetPosition() + wxPoint(50, 50), wxSize(300, 700));
    ramWatchFrame->Show();
}

void MainFrame::OnExportFiles(wxCommandEvent& event)
{
    app->cdi->ExportFiles();
}

void MainFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("CeDImu is an open-source Philips CD-I emulator created by Stovent.", "About CeDImu", wxOK);
}
