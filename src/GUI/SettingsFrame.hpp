#ifndef GUI_SETTINGSFRAME_HPP
#define GUI_SETTINGSFRAME_HPP

class MainFrame;

#include <wx/frame.h>

class SettingsFrame : public wxFrame
{
public:
    MainFrame* m_mainFrame;

    int m_keyUp;
    int m_keyRight;
    int m_keyDown;
    int m_keyLeft;
    int m_key1;
    int m_key2;
    int m_key12;

    SettingsFrame() = delete;
    SettingsFrame(MainFrame* parent);
    ~SettingsFrame();
};

#endif // GUI_SETTINGSFRAME_HPP
