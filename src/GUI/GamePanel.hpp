#ifndef GUI_GAMEPANEL_HPP
#define GUI_GAMEPANEL_HPP

class MainFrame;
#include "../CeDImu.hpp"

#include <wx/image.h>
#include <wx/dc.h>
#include <wx/panel.h>

#include <mutex>

class GamePanel : public wxPanel
{
public:
    MainFrame* m_mainFrame;
    CeDImu& m_cedimu;
    std::mutex m_screenMutex;
    wxImage m_screen;
    bool m_stopOnNextFrame;

    GamePanel() = delete;
    GamePanel(MainFrame* parent, CeDImu& cedimu);
    ~GamePanel();

    void Reset();
    bool SaveScreenshot(const std::string& file);

    void DrawScreen(wxDC& dc);
    void OnPaintEvent(wxPaintEvent&);

    void OnKeyDown(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // GUI_GAMEPANEL_HPP
