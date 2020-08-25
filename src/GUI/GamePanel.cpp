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
    wxImage screen = app->cdi->board->vdsc->GetScreen();
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
    wxImage screen = app->cdi->board->vdsc->GetScreen();
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
        if(app->cdi->board)
            mainFrame->Pause();
        break;

    case 'Z':
        if(!app->cdi->board) break;
        app->cdi->board->vdsc->StopOnNextFrame();
        if(!app->cdi->board->cpu->IsRunning())
            app->StartGameThread();
        break;

    case 'E':
        if(app->cdi->board && app->mainFrame->pauseItem->IsChecked())
            app->cdi->board->cpu->Run(false);
        break;
    case 'M':
        app->DecreaseEmulationSpeed();
        break;
    case 61:
        app->IncreaseEmulationSpeed();
        break;
    }
}
