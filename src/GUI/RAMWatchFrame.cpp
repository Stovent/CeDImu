#include "RAMWatchFrame.hpp"
#include <wx/msgdlg.h>

wxBEGIN_EVENT_TABLE(RAMWatchFrame, wxFrame)
    EVT_TIMER(wxID_ANY, RAMWatchFrame::RefreshLoop)
    EVT_CLOSE(RAMWatchFrame::OnClose)
wxEND_EVENT_TABLE()

RAMWatchFrame::RAMWatchFrame(VDSC* vds, MainFrame* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "RAM Watch", pos, size)
{
    vdsc = vds;
    mainFrame = parent;

    grid = new wxGrid(this, wxID_ANY);
    grid->CreateGrid(1024*1024, 2);
    grid->SetColLabelValue(0, "Address");
    grid->SetColLabelValue(1, "Value");
    grid->EnableEditing(false);
    for(int i = 0; i < 1024*1024; i++)
    {
        grid->SetCellValue(std::to_string(i), i, 0);
        grid->SetCellValue(std::to_string(vdsc->memory[i]), i, 1);
    }

    renderTimer = new wxTimer(this);
    renderTimer->Start(16);
}

RAMWatchFrame::~RAMWatchFrame()
{
    mainFrame->ramWatchFrame = nullptr;
    delete grid;
    delete renderTimer;
}

void RAMWatchFrame::OnClose(wxCloseEvent& event)
{
    renderTimer->Stop();
    Destroy();
}

void RAMWatchFrame::RefreshLoop(wxTimerEvent& event)
{
    PaintEvent();
}

void RAMWatchFrame::PaintEvent()
{

}
