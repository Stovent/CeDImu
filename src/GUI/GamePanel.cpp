#include "GamePanel.hpp"

wxBEGIN_EVENT_TABLE(GamePanel, wxPanel)
    EVT_KEY_DOWN(GamePanel::OnKeyDown)
wxEND_EVENT_TABLE()

GamePanel::GamePanel(MainFrame* parent, CeDImu* appp) : wxPanel(parent)
{
    mainFrame = parent;
    app = appp;
    screen.Create(1, 1);
}

GamePanel::~GamePanel()
{
}

void GamePanel::RefreshLoop(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    dc.Clear();
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.DrawRectangle(0, 0, mainFrame->GetClientSize().x, mainFrame->GetClientSize().y);
    dc.DrawBitmap(wxBitmap(screen.Scale(mainFrame->GetClientSize().x, mainFrame->GetClientSize().y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
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
        break;

    case 'Z':
        if(!app->cpu) break;
        app->vdsc->stopOnNextCompletedFrame = true;
        if(!app->cpu->run)
            app->StartGameThread();
        break;

    case 'E':
        if(app->cpu && app->mainFrame->pauseItem->IsChecked())
            app->cpu->Run(false);
        break;
    }
}
