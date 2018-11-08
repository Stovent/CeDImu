#include "MainFrame.hpp"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(IDFileOpenROM,   MainFrame::OnOpenROM)
    EVT_MENU(IDFileOpenMovie, MainFrame::OnOpenMovie)
    EVT_MENU(wxID_EXIT,  MainFrame::OnExit)

    //EVT_MENU(IDCdiExportDiskSectorsInfo, CDI::ExportDiskSectorsInfo)
    //EVT_MENU(IDCdiExportAudio, CDI::ExportAudio)
    //EVT_MENU(IDCdiExportFiles, CDI::ExportFiles)
    //EVT_MENU(IDCdiExportVideo, CDI::ExportVideo)

    EVT_MENU(IDHelpAbout, MainFrame::OnAbout)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(CeDImu* appp, const wxString& title, const wxPoint& pos, const wxSize& size) : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    app = appp;
    gamePanel = new GamePanel(this, appp);

    CreateMenuBar();

    CreateStatusBar();
    SetStatusText("CeDImu");
}

void MainFrame::CreateMenuBar()
{
    wxMenu* file = new wxMenu;
    file->Append(IDFileOpenROM, "Open ROM\tCtrl+O", "Choose the ROM to load");
    file->AppendSeparator();
    file->Append(IDFileSaveStateAs, "Save state as", "Save a savestate to a file");
    file->Append(IDFileLoadStateFrom, "Load state from", "Load a savestate from a file");
    file->AppendSeparator();
    file->Append(IDFileOpenMovie, "Load Movie\tCtrl+L", "Load an input file");
    file->AppendSeparator();
    file->Append(IDFileCloseROM, "Close ROM\tCtrl+Maj+O", "Close the ROM currently playing");
    file->Append(wxID_EXIT);

    wxMenu* tools = new wxMenu;
    tools->Append(IDToolsDisassembler, "Disassembler", "The instructions that have been executed on the last frame");
    tools->Append(IDToolsCPUViewer, "CPU Viewer", "View the internal state of the CPU");
    tools->Append(IDToolsRAMSearch, "RAM Search", "View the entire RAM and make researches in it");
    tools->Append(IDToolsRAMWatch, "RAM Watch", "Keep track of the  watching RAM values");

    wxMenu* cdiExport = new wxMenu;
    cdiExport->Append(IDCdiExportDiskSectorsInfo, "Disk sectors info");
    cdiExport->Append(IDCdiExportFiles, "Files", "Export all the files from the disk");
    cdiExport->Append(IDCdiExportAudio, "Audio");
    cdiExport->Append(IDCdiExportVideo, "Video");

    wxMenu* cdi = new wxMenu;
    cdi->Append(IDCdiDiskLabel, "Disk Label", "See the Disk Label info");
    cdi->AppendSeparator();
    cdi->AppendSubMenu(cdiExport, "Export", "Export data, audio, video and various information from the disk");

    wxMenu* help = new wxMenu;
    help->Append(IDHelpAbout, "About");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(file, "File");
    menuBar->Append(tools, "Tools");
    menuBar->Append(cdi, "CD-I");
    menuBar->Append(help, "Help");

    SetMenuBar(menuBar);
}

void MainFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("CeDImu is an open-source Philips CD-I emulator created by Stovent.", "About CeDImu", wxOK );
}

void MainFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MainFrame::OnOpenMovie(wxCommandEvent& event)
{
    wxMessageBox("OnOpenMovie");
}

void MainFrame::OnOpenROM(wxCommandEvent& event)
{
    wxMessageBox("OnOpenROM");
}
