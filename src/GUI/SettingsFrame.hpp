#ifndef SETTINGSFRAME_HPP
#define SETTINGSFRAME_HPP

class MainFrame;

#include <wx/frame.h>

class SettingsFrame : public wxFrame
{
public:
    MainFrame* mainFrame;

    SettingsFrame(MainFrame* parent);
    ~SettingsFrame();
};

#endif // SETTINGSFRAME_HPP
