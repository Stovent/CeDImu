#ifndef GUI_SETTINGSFRAME_HPP
#define GUI_SETTINGSFRAME_HPP

class MainFrame;
#include "../Config.hpp"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/frame.h>
#include <wx/listbox.h>
#include <wx/radiobut.h>
#include <wx/textctrl.h>

#include <vector>

class SettingsFrame : public wxFrame
{
public:
    MainFrame* m_mainFrame;

    std::vector<Config::BiosConfig> m_biosConfigs;

    wxTextCtrl* m_discPath;

    int m_lastSelection;
    wxListBox* m_biosList;
    wxTextCtrl* m_biosPath;
    wxButton* m_biosSelect;
    wxTextCtrl* m_nvramFileName;
    wxChoice* m_boardChoice;
    wxCheckBox* m_palCheckBox;
    wxCheckBox* m_nvramCheckBox;
    wxTextCtrl* m_initialTime;

    int m_keyUp;
    int m_keyRight;
    int m_keyDown;
    int m_keyLeft;
    int m_key1;
    int m_key2;
    int m_key12;

    SettingsFrame() = delete;
    explicit SettingsFrame(MainFrame* parent);
    ~SettingsFrame();

    void OnSaveConfig(wxEvent&);
    void OnNewConfig(wxCommandEvent&);
    void OnDeleteConfig(wxCommandEvent&);
    void OnSelectBios(wxCommandEvent& event);

private:
    void LoadSelection();
    void SaveSelection();
    void CheckControls();

    wxDECLARE_EVENT_TABLE();
};

#endif // GUI_SETTINGSFRAME_HPP
