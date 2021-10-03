#include "GamePanel.hpp"
#include "CPUViewer.hpp"
#include "MainFrame.hpp"
#include "VDSCViewer.hpp"

#include "../Config.hpp"

#include <wx/dcclient.h>

wxBEGIN_EVENT_TABLE(GamePanel, wxPanel)
    EVT_PAINT(GamePanel::OnPaintEvent)
    EVT_KEY_DOWN(GamePanel::OnKeyDown)
    EVT_KEY_UP(GamePanel::OnKeyUp)
wxEND_EVENT_TABLE()

GamePanel::GamePanel(MainFrame* parent, CeDImu& cedimu) :
    wxPanel(parent),
    m_mainFrame(parent),
    m_cedimu(cedimu),
    m_screen(0, 0),
    m_stopOnNextFrame(false)
{
    SetDoubleBuffered(true);

    m_cedimu.m_cdi.callbacks.SetOnFrameCompleted([this] (const Plane& plane) {
        if(m_mainFrame->m_cpuViewer)
            m_mainFrame->m_cpuViewer->m_flushInstructions = true;

        if(m_mainFrame->m_vdscViewer)
            m_mainFrame->m_vdscViewer->m_flushIcadca = true;

        if(this->m_stopOnNextFrame)
        {
            this->m_cedimu.m_cdi.board->cpu.Stop(false);
            this->m_mainFrame->m_pauseMenuItem->Check();
        }

        std::lock_guard<std::mutex> __(this->m_screenMutex);
        if(this->m_screen.Create(plane.width, plane.height))
        {
            memcpy(this->m_screen.GetData(), plane.data(), plane.width * plane.height * 3);
            this->Refresh();
        }
    });
}

GamePanel::~GamePanel()
{
    m_cedimu.m_cdi.callbacks.SetOnFrameCompleted(nullptr);
}

void GamePanel::Reset()
{
    std::lock_guard<std::mutex> lock(m_screenMutex);
    m_screen = wxImage(0, 0);
    Refresh();
}

bool GamePanel::SaveScreenshot(const std::string& path)
{
    std::lock_guard<std::mutex> lock(m_cedimu.m_cdiBoardMutex);
    if(!m_cedimu.m_cdi.board)
        return false;

    std::lock_guard<std::mutex> lock2(m_screenMutex);
    uint32_t fc = m_cedimu.m_cdi.board->GetTotalFrameCount();
    return m_screen.SaveFile(path + "/frame_" + std::to_string(fc) + ".png", wxBITMAP_TYPE_PNG);
}

void GamePanel::DrawScreen(wxDC& dc)
{
    dc.Clear();
    std::lock_guard<std::mutex> lock(m_screenMutex);
    if(m_screen.IsOk())
    {
        const wxSize size = GetClientSize();
        if(size.x > 0 && size.y > 0)
        {
            wxBitmap screen(m_screen.Scale(size.x, size.y, wxIMAGE_QUALITY_NEAREST));
            if(screen.IsOk())
                dc.DrawBitmap(screen, 0, 0);
        }
    }
}

void GamePanel::OnPaintEvent(wxPaintEvent&)
{
    wxPaintDC dc(this);
    DrawScreen(dc);
}

void GamePanel::OnKeyDown(wxKeyEvent& event)
{
    std::lock_guard<std::mutex> lock(m_cedimu.m_cdiBoardMutex);
    int keyCode = event.GetKeyCode();
    if(keyCode == Config::keyUp)
    {
        if(m_cedimu.m_cdi.board)
            m_cedimu.m_cdi.board->slave->pointingDevice->SetUp(true);
    }
    else if(keyCode == Config::keyRight)
    {
        if(m_cedimu.m_cdi.board)
            m_cedimu.m_cdi.board->slave->pointingDevice->SetRight(true);
    }
    else if(keyCode == Config::keyDown)
    {
        if(m_cedimu.m_cdi.board)
            m_cedimu.m_cdi.board->slave->pointingDevice->SetDown(true);
    }
    else if(keyCode == Config::keyLeft)
    {
        if(m_cedimu.m_cdi.board)
            m_cedimu.m_cdi.board->slave->pointingDevice->SetLeft(true);
    }
    else if(keyCode == Config::key1)
    {
        if(m_cedimu.m_cdi.board)
            m_cedimu.m_cdi.board->slave->pointingDevice->SetButton1(true);
    }
    else if(keyCode == Config::key2)
    {
        if(m_cedimu.m_cdi.board)
            m_cedimu.m_cdi.board->slave->pointingDevice->SetButton2(true);
    }
    else if(keyCode == Config::key12)
    {
        if(m_cedimu.m_cdi.board)
        {
            m_cedimu.m_cdi.board->slave->pointingDevice->SetButton1(true);
            m_cedimu.m_cdi.board->slave->pointingDevice->SetButton2(true);
        }
    }
    else
        event.Skip();
}

void GamePanel::OnKeyUp(wxKeyEvent& event)
{
    std::lock_guard<std::mutex> lock(m_cedimu.m_cdiBoardMutex);
    int keyCode = event.GetKeyCode();
    if(keyCode == Config::keyUp)
    {
        if(m_cedimu.m_cdi.board)
            m_cedimu.m_cdi.board->slave->pointingDevice->SetUp(false);
    }
    else if(keyCode == Config::keyRight)
    {
        if(m_cedimu.m_cdi.board)
            m_cedimu.m_cdi.board->slave->pointingDevice->SetRight(false);
    }
    else if(keyCode == Config::keyDown)
    {
        if(m_cedimu.m_cdi.board)
            m_cedimu.m_cdi.board->slave->pointingDevice->SetDown(false);
    }
    else if(keyCode == Config::keyLeft)
    {
        if(m_cedimu.m_cdi.board)
            m_cedimu.m_cdi.board->slave->pointingDevice->SetLeft(false);
    }
    else if(keyCode == Config::key1)
    {
        if(m_cedimu.m_cdi.board)
            m_cedimu.m_cdi.board->slave->pointingDevice->SetButton1(false);
    }
    else if(keyCode == Config::key2)
    {
        if(m_cedimu.m_cdi.board)
            m_cedimu.m_cdi.board->slave->pointingDevice->SetButton2(false);
    }
    else if(keyCode == Config::key12)
    {
        if(m_cedimu.m_cdi.board)
        {
            m_cedimu.m_cdi.board->slave->pointingDevice->SetButton1(false);
            m_cedimu.m_cdi.board->slave->pointingDevice->SetButton2(false);
        }
    }
    else
        event.Skip();
}
