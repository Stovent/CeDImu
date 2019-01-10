#include "MainFrame.hpp"

#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/position.h>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(IDOnOpenROM,   MainFrame::OnOpenROM)
    EVT_MENU(IDOnCloseROM,   MainFrame::OnCloseROM)
    EVT_MENU(wxID_EXIT,  MainFrame::OnExit)
    EVT_MENU(IDOnDisassembler, MainFrame::OnDisassembler)
    EVT_MENU(IDOnAbout, MainFrame::OnAbout)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(CeDImu* appp, const wxString& title, const wxPoint& pos, const wxSize& size) : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    app = appp;
    gamePanel = new GamePanel(this, appp);
    disassemblerFrame = nullptr;

    CreateMenuBar();

    CreateStatusBar();
    SetStatusText("CeDImu");
}

void MainFrame::CreateMenuBar()
{
    wxMenu* file = new wxMenu;
    file->Append(IDOnOpenROM, "Open ROM\tCtrl+O", "Choose the ROM to load");
    file->AppendSeparator();
    file->Append(IDOnCloseROM, "Close ROM\tCtrl+Maj+O", "Close the ROM currently playing");
    file->Append(wxID_EXIT);

    wxMenu* tools = new wxMenu;
    tools->Append(IDOnDisassembler, "Disassembler\tCtrl+D");

    wxMenu* help = new wxMenu;
    help->Append(IDOnAbout, "About");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(file, "File");
    menuBar->Append(tools, "Tools");
    menuBar->Append(help, "Help");

    SetMenuBar(menuBar);
}

void MainFrame::OnOpenROM(wxCommandEvent& event)
{
    app->StartGameThread();
}

void MainFrame::OnCloseROM(wxCommandEvent& event)
{
    app->StopGameThread();
}

void MainFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void MainFrame::OnDisassembler(wxCommandEvent& event)
{
    if(disassemblerFrame == nullptr)
        disassemblerFrame = new DisassemblerFrame(*(app->cpu), this, this->GetPosition() + wxPoint(0, 50), wxSize(600, 400));
    disassemblerFrame->Show();
}

void MainFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("CeDImu is an open-source Philips CD-I emulator created by Stovent.", "About CeDImu", wxOK);
}
