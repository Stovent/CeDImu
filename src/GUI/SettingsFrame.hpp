#ifndef GUI_SETTINGSFRAME_HPP
#define GUI_SETTINGSFRAME_HPP

class MainFrame;

#include <wx/frame.h>

class SettingsFrame : public wxFrame
{
public:
    MainFrame* m_mainFrame;

    SettingsFrame(MainFrame* parent);
    ~SettingsFrame();
};

#endif // GUI_SETTINGSFRAME_HPP
