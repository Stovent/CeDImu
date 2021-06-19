#include "GamePanel.hpp"

wxBEGIN_EVENT_TABLE(GamePanel, wxPanel)
    EVT_KEY_DOWN(GamePanel::OnKeyDown)
    EVT_KEY_UP(GamePanel::OnKeyUp)
wxEND_EVENT_TABLE()

GamePanel::GamePanel(MainFrame* parent, CeDImu& appp) : wxPanel(parent), app(appp)
{
    mainFrame = parent;
}

GamePanel::~GamePanel()
{
    app.cdi.board->SetOnFrameCompletedCallback(nullptr);
}

void GamePanel::RefreshLoop(wxPaintEvent& event)
{
    Plane p = app.cdi.board->GetScreen();
    if(p.pixels == nullptr)
        return;

    wxImage screen(p.width, p.height);
    memcpy(screen.GetData(), p.pixels, p.width * p.height * 3);

    wxPaintDC dc(this);
    dc.Clear();
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.DrawRectangle(0, 0, mainFrame->GetClientSize().x, mainFrame->GetClientSize().y);
    dc.DrawBitmap(wxBitmap(screen.Scale(mainFrame->GetClientSize().x, mainFrame->GetClientSize().y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
}

void GamePanel::RefreshScreen()
{
    Plane p = app.cdi.board->GetScreen();
    if(p.pixels == nullptr)
        return;

    frameWidth = p.width;
    frameHeight = p.height;
    wxImage screen(p.width, p.height);
    memcpy(screen.GetData(), p.pixels, p.width * p.height * 3);

    wxClientDC dc(this);
    dc.DrawBitmap(wxBitmap(screen.Scale(mainFrame->GetClientSize().x, mainFrame->GetClientSize().y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
}

void GamePanel::OnKeyDown(wxKeyEvent& event)
{
    switch(event.GetKeyCode())
    {
    // Shortcuts
    case 'A':
        mainFrame->Pause();
        break;

    case 'Z':
        if(!app.cdi.board)
            break;
        app.cdi.board->StopOnNextFrame();
        if(!app.cdi.board->cpu.IsRunning())
            app.StartGameThread();
        break;

    case 'E':
        if(app.cdi.board && app.mainFrame->pauseItem->IsChecked())
            app.cdi.board->cpu.Run(false);
        break;
    case 'M':
        app.DecreaseEmulationSpeed();
        break;
    case '=':
        app.IncreaseEmulationSpeed();
        break;

    // Controller
    case 'O':
        if(app.cdi.board)
            app.cdi.board->slave->pointingDevice->SetButton1(true);
        break;

    case 'P':
        if(app.cdi.board)
            app.cdi.board->slave->pointingDevice->SetButton2(true);
        break;

    case 'D':
        if(app.cdi.board)
            app.cdi.board->slave->pointingDevice->SetLeft(true);
        break;

    case 'R':
        if(app.cdi.board)
            app.cdi.board->slave->pointingDevice->SetUp(true);
        break;

    case 'G':
        if(app.cdi.board)
            app.cdi.board->slave->pointingDevice->SetRight(true);
        break;

    case 'F':
        if(app.cdi.board)
            app.cdi.board->slave->pointingDevice->SetDown(true);
        break;
    }
}

void GamePanel::OnKeyUp(wxKeyEvent& event)
{
    switch(event.GetKeyCode())
    {
    // Controller
    case 'O':
        if(app.cdi.board)
            app.cdi.board->slave->pointingDevice->SetButton1(false);
        break;

    case 'P':
        if(app.cdi.board)
            app.cdi.board->slave->pointingDevice->SetButton2(false);
        break;

    case 'D':
        if(app.cdi.board)
            app.cdi.board->slave->pointingDevice->SetLeft(false);
        break;

    case 'R':
        if(app.cdi.board)
            app.cdi.board->slave->pointingDevice->SetUp(false);
        break;

    case 'G':
        if(app.cdi.board)
            app.cdi.board->slave->pointingDevice->SetRight(false);
        break;

    case 'F':
        if(app.cdi.board)
            app.cdi.board->slave->pointingDevice->SetDown(false);
        break;
    }
}
