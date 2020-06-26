#include "GamePanel.hpp"

wxBEGIN_EVENT_TABLE(GamePanel, wxPanel)
    EVT_KEY_DOWN(GamePanel::OnKeyDown)
wxEND_EVENT_TABLE()

GamePanel::GamePanel(MainFrame* parent, CeDImu* appp) : wxPanel(parent)
{
    mainFrame = parent;
    app = appp;
}

GamePanel::~GamePanel()
{
}

void GamePanel::RefreshLoop(wxPaintEvent& event)
{
    wxImage screen = app->vdsc->GetScreen();
    if(!screen.IsOk())
        return;

    wxPaintDC dc(this);
    dc.Clear();
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.DrawRectangle(0, 0, mainFrame->GetClientSize().x, mainFrame->GetClientSize().y);
    dc.DrawBitmap(wxBitmap(screen.Scale(mainFrame->GetClientSize().x, mainFrame->GetClientSize().y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
}

void GamePanel::RefreshScreen()
{
    wxImage screen = app->vdsc->GetScreen();
    if(!screen.IsOk())
        return;

    wxClientDC dc(this);
    dc.DrawBitmap(wxBitmap(screen.Scale(mainFrame->GetClientSize().x, mainFrame->GetClientSize().y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
}

void GamePanel::OnKeyDown(wxKeyEvent& event)
{
    switch(event.GetKeyCode())
    {
    case 'A':
        if(app->cpu)
            mainFrame->Pause();
        break;

    case 'Z':
        if(!app->cpu) break;
        app->vdsc->StopOnNextFrame();
        if(!app->cpu->IsRunning())
            app->StartGameThread();
        break;

    case 'E':
        if(app->cpu && app->mainFrame->pauseItem->IsChecked())
            app->cpu->Run(false);
        break;
    case 'M':
        app->DecreaseEmulationSpeed();
        break;
    case 61:
        app->IncreaseEmulationSpeed();
        break;
    }
}
