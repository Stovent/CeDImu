#ifndef GUI_SETTINGSFRAME_HPP
#define GUI_SETTINGSFRAME_HPP

class MainFrame;

#include <wx/checkbox.h>
#include <wx/frame.h>
#include <wx/radiobut.h>
#include <wx/textctrl.h>

#include <vector>

struct BiosWidgets
{
    wxTextCtrl* name;
    wxTextCtrl* biosPath;
    wxTextCtrl* nvramFileName;
    wxRadioButton* boardAuto;
    wxRadioButton* boardMono34Roboco;
    wxCheckBox* has32KbNvram;
    wxCheckBox* pal;
    wxTextCtrl* initialTime;
};

class SettingsFrame : public wxFrame
{
public:
    MainFrame* m_mainFrame;

    std::vector<BiosWidgets> m_biosWidgets;

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
