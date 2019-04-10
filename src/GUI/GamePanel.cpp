#include "GamePanel.hpp"

wxBEGIN_EVENT_TABLE(GamePanel, wxPanel)
    EVT_KEY_DOWN(GamePanel::OnKeyDown)
wxEND_EVENT_TABLE()

GamePanel::GamePanel(MainFrame* parent, CeDImu* appp) : wxPanel(parent)
{
    mainFrame = parent;
    app = appp;
}

void GamePanel::OnKeyDown(wxKeyEvent& event)
{
    switch(event.GetKeyCode())
    {
    case 'A':
        if(app->mainFrame->pause->IsChecked())
        {
            app->mainFrame->pause->Check(false);
            app->mainFrame->SetStatusText("Running");
        }
        else
        {
            app->mainFrame->pause->Check(true);
            app->mainFrame->SetStatusText("Pause");
        }
        app->mainFrame->OnPause();
    break;

    case 'E':
        if(app->mainFrame->pause->IsChecked())
            app->cpu->SingleStep();
    break;
    }
}
