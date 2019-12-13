#include "GamePanel.hpp"

wxBEGIN_EVENT_TABLE(GamePanel, wxPanel)
    EVT_TIMER(wxID_ANY, GamePanel::RefreshLoop)
    EVT_KEY_DOWN(GamePanel::OnKeyDown)
wxEND_EVENT_TABLE()

GamePanel::GamePanel(MainFrame* parent, CeDImu* appp) : wxPanel(parent)
{
    mainFrame = parent;
    app = appp;
    screen.Create(1, 1);
    oldInstCount = 0;
    oldFrameCount = 0;

    renderTimer = new wxTimer(this);
    renderTimer->Start(1000);
}

GamePanel::~GamePanel()
{
    renderTimer->Stop();
    delete renderTimer;
}

void GamePanel::RefreshLoop(wxTimerEvent& event)
{
    wxClientDC dc(this);
    dc.Clear();
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.DrawRectangle(0, 0, mainFrame->GetClientSize().x, mainFrame->GetClientSize().y);
    dc.DrawBitmap(wxBitmap(screen.Scale(mainFrame->GetClientSize().x, mainFrame->GetClientSize().y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
    if(!mainFrame->pause->IsChecked())
        DrawTextInfo(dc);
}

void GamePanel::DrawTextInfo(wxClientDC& dc)
{
    if(app->vdsc)
    {
        mainFrame->SetTitleInfo(app->vdsc->totalFrameCount - oldFrameCount);
        oldFrameCount = app->vdsc->totalFrameCount;
    }
}

void GamePanel::RefreshScreen(const wxImage& img)
{
    wxClientDC dc(this);
    screen = img.Copy();
    dc.DrawBitmap(wxBitmap(screen.Scale(mainFrame->GetClientSize().x, mainFrame->GetClientSize().y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
}

void GamePanel::OnKeyDown(wxKeyEvent& event)
{
    switch(event.GetKeyCode())
    {
    case 'A':
        if(!app->cpu) break;
        mainFrame->Pause();
        app->cpu->instructionsBufferChanged = true;
        break;

    case 'Z':
        if(!app->cpu) break;
        app->vdsc->stopOnNextCompletedFrame = true;
        if(!app->cpu->run)
            app->StartGameThread();
        break;

    case 'E':
        if(app->cpu && app->mainFrame->pause->IsChecked())
            app->cpu->SingleStep();
        break;
    }
}
