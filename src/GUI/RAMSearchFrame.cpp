#include "RAMSearchFrame.hpp"
#include <wx/msgdlg.h>

wxBEGIN_EVENT_TABLE(RAMSearchFrame, wxFrame)
    EVT_TIMER(wxID_ANY, RAMSearchFrame::RefreshLoop)
    EVT_CLOSE(RAMSearchFrame::OnClose)
wxEND_EVENT_TABLE()

RAMSearchFrame::RAMSearchFrame(VDSC* vds, MainFrame* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "RAM Search", pos, size)
{
    vdsc = vds;
    mainFrame = parent;

    ramSearchList = new RAMSearchList(this);

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(ramSearchList, 1, wxEXPAND, 1);
    SetSizer(sizer);

    renderTimer = new wxTimer(this);
    renderTimer->Start(16);
}

RAMSearchFrame::~RAMSearchFrame()
{
    mainFrame->ramSearchFrame = nullptr;
    delete ramSearchList;
    delete renderTimer;
}

void RAMSearchFrame::OnClose(wxCloseEvent& event)
{
    renderTimer->Stop();
    Destroy();
}

void RAMSearchFrame::RefreshLoop(wxTimerEvent& event)
{
    PaintEvent();
}

void RAMSearchFrame::PaintEvent()
{

}
