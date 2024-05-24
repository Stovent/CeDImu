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

GamePanel::GamePanel(MainFrame* parent, CeDImu& cedimu)
    : wxPanel(parent)
    , m_mainFrame(parent)
    , m_cedimu(cedimu)
    , m_screen(0, 0)
    , m_stopOnNextFrame(false)
{
    SetDoubleBuffered(true);

    m_cedimu.SetOnFrameCompleted([this] (const Video::Plane& plane) {
        if(m_mainFrame->m_cpuViewer != nullptr)
            m_mainFrame->m_cpuViewer->m_flushInstructions = true;

        if(m_mainFrame->m_vdscViewer != nullptr)
            m_mainFrame->m_vdscViewer->m_flushIcadca = true;

        if(this->m_stopOnNextFrame)
        {
            this->m_cedimu.m_cdi->m_cpu.Stop(false);
            this->m_mainFrame->m_pauseMenuItem->Check();
        }

        std::lock_guard<std::mutex> __(this->m_screenMutex);
        if(this->m_screen.Create(plane.m_width, plane.m_height))
        {
            memcpy(this->m_screen.GetData(), plane.data(), plane.m_width * plane.m_height * 3);
            this->Refresh();
        }
    });
}

GamePanel::~GamePanel()
{
    m_cedimu.SetOnFrameCompleted(nullptr);
}

void GamePanel::Reset()
{
    std::lock_guard<std::mutex> lock(m_screenMutex);
    m_screen = wxImage(0, 0);
    Refresh();
}

bool GamePanel::SaveScreenshot(const std::string& file)
{
    std::lock_guard<std::recursive_mutex> lock(m_cedimu.m_cdiMutex);
    if(!m_cedimu.m_cdi)
        return false;

    std::lock_guard<std::mutex> lock2(m_screenMutex);
    return m_screen.SaveFile(file, wxBITMAP_TYPE_PNG);
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
    std::lock_guard<std::recursive_mutex> lock(m_cedimu.m_cdiMutex);
    if(!m_cedimu.m_cdi)
        return event.Skip();

    int keyCode = event.GetKeyCode();
    if(keyCode == Config::keyUp)
    {
        m_cedimu.m_cdi->m_slave->pointingDevice.SetUp(true);
    }
    else if(keyCode == Config::keyRight)
    {
        m_cedimu.m_cdi->m_slave->pointingDevice.SetRight(true);
    }
    else if(keyCode == Config::keyDown)
    {
        m_cedimu.m_cdi->m_slave->pointingDevice.SetDown(true);
    }
    else if(keyCode == Config::keyLeft)
    {
        m_cedimu.m_cdi->m_slave->pointingDevice.SetLeft(true);
    }
    else if(keyCode == Config::key1)
    {
        m_cedimu.m_cdi->m_slave->pointingDevice.SetButton1(true);
    }
    else if(keyCode == Config::key2)
    {
        m_cedimu.m_cdi->m_slave->pointingDevice.SetButton2(true);
    }
    else if(keyCode == Config::key12)
    {
        m_cedimu.m_cdi->m_slave->pointingDevice.SetButton12(true);
    }
    else
        event.Skip();
}

void GamePanel::OnKeyUp(wxKeyEvent& event)
{
    std::lock_guard<std::recursive_mutex> lock(m_cedimu.m_cdiMutex);
    if(!m_cedimu.m_cdi)
        return event.Skip();

    int keyCode = event.GetKeyCode();
    if(keyCode == Config::keyUp)
    {
        m_cedimu.m_cdi->m_slave->pointingDevice.SetUp(false);
    }
    else if(keyCode == Config::keyRight)
    {
        m_cedimu.m_cdi->m_slave->pointingDevice.SetRight(false);
    }
    else if(keyCode == Config::keyDown)
    {
        m_cedimu.m_cdi->m_slave->pointingDevice.SetDown(false);
    }
    else if(keyCode == Config::keyLeft)
    {
        m_cedimu.m_cdi->m_slave->pointingDevice.SetLeft(false);
    }
    else if(keyCode == Config::key1)
    {
        m_cedimu.m_cdi->m_slave->pointingDevice.SetButton1(false);
    }
    else if(keyCode == Config::key2)
    {
        m_cedimu.m_cdi->m_slave->pointingDevice.SetButton2(false);
    }
    else if(keyCode == Config::key12)
    {
        m_cedimu.m_cdi->m_slave->pointingDevice.SetButton12(false);
    }
    else
        event.Skip();
}
