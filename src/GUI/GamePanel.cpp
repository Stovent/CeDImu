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
    if(app->cpu)
    {
        dc.DrawText("inst per seconds: " + std::to_string(app->cpu->count - oldInstCount), wxPoint(0, 0));
        oldInstCount = app->cpu->count;
    }
}

void GamePanel::RefreshScreen(const wxImage& img)
{
    wxClientDC dc(this);
    screen = img.Copy();
    dc.DrawBitmap(wxBitmap(img.Scale(app->mainFrame->GetClientSize().x, app->mainFrame->GetClientSize().y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
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
        break;

    case 'E':
        if(app->cpu && app->mainFrame->pause->IsChecked())
            app->cpu->SingleStep();
        break;
    }
}
