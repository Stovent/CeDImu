#include "GamePanel.hpp"
#include "CPUViewer.hpp"
#include "MainFrame.hpp"
#include "VDSCViewer.hpp"

#include "../Config.hpp"
#include "../export.hpp"

#include <wx/dcclient.h>

#if _WIN32
#include <windows.h>
#endif

#include <thread>

wxBEGIN_EVENT_TABLE(GamePanel, wxPanel)
    EVT_PAINT(GamePanel::OnPaintEvent)
    EVT_KEY_DOWN(GamePanel::OnKeyDown)
    EVT_KEY_UP(GamePanel::OnKeyUp)
wxEND_EVENT_TABLE()

// /** \brief Spin-loop sleep for accurate timing.
//  * High CPU usage (thread always at 100%), looses ~5% max speed perfs.
//  */
// template<typename CLOCK, typename TIME_POINT = CLOCK::time_point>
// [[maybe_unused]]
// static void sleepUntilSpin(const TIME_POINT& target)
// {
//     while(CLOCK::now() < target);
// }

// /** \brief More accurate sleep function on Windows.
//  * Slight loss of max speed performances.
//  */
// template<typename CLOCK, typename TIME_POINT = CLOCK::time_point>
// static void sleepUntil(const TIME_POINT& target)
// {
// #if _WIN32
//     while(CLOCK::now() < target)
//         Sleep(1);
// #else
//     std::this_thread::sleep_until(target);
// #endif
// }

GamePanel::GamePanel(MainFrame* parent, CeDImu& cedimu)
    : wxPanel(parent)
    , m_mainFrame(parent)
    , m_cedimu(cedimu)
{
    SetDoubleBuffered(true);
#if _WIN32
    timeBeginPeriod(1); // Request 1ms sleep granularity
#endif

    m_cedimu.SetOnFrameCompleted([this] (const Video::Plane& plane) {
        if(m_mainFrame->m_cpuViewer != nullptr)
            m_mainFrame->m_cpuViewer->m_flushInstructions = true;

        if(m_mainFrame->m_vdscViewer != nullptr)
            m_mainFrame->m_vdscViewer->m_flushIcadca = true;

        if(this->m_stopOnNextFrame)
        {
            this->m_cedimu.m_cdi->Stop(false);
            this->m_mainFrame->m_pauseMenuItem->Check();
        }

        {
            std::lock_guard<std::mutex> __(this->m_screenMutex);
            if(this->m_screen.Create(plane.m_width, plane.m_height))
            {
                splitARGB(plane.GetSpan(), nullptr, this->m_screen.GetData());
                this->Refresh();
            }
        }

        // Pause for emulation speed.
        const Duration delta{this->m_cedimu.GetEmulationSpeedFrameDelay()};
        const TimePoint target = m_lastFrame + delta;
        const TimePoint now = Clock::now();

        if(target <= now) // Emulation is faster than UI rendering.
        {
            m_lastFrame = now;
        }
        else
        {
            m_lastFrame = target;
            std::this_thread::sleep_until(target);
            // sleepUntilSpin<Clock>(target);
            // sleepUntil<Clock>(target);
        }
    });
}

GamePanel::~GamePanel()
{
    m_cedimu.SetOnFrameCompleted(nullptr);
#if _WIN32
    timeEndPeriod(1);
#endif
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
