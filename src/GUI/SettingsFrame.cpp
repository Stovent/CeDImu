#include "SettingsFrame.hpp"
#include "MainFrame.hpp"
#include "../Config.hpp"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/hyperlink.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/aui/auibook.h>

#include <filesystem>

#define CREATE_CONTROL_BUTTON(k, label)     wxBoxSizer* key##k##Sizer = new wxBoxSizer(wxHORIZONTAL); \
    controlsSizer->Add(key##k##Sizer, wxSizerFlags().DoubleBorder()); \
    wxStaticText* key##k##Text = new wxStaticText(controlsPanel, wxID_ANY, label); \
    key##k##Sizer->Add(key##k##Text, wxSizerFlags().Border(wxRIGHT, 20).Align(wxALIGN_CENTER_VERTICAL)); \
    wxButton* key##k##Button = new wxButton(controlsPanel, wxID_ANY, wxAcceleratorEntry(0, m_key##k).ToString(), wxDefaultPosition, wxSize(100, 25)); \
    key##k##Button->Bind(wxEVT_BUTTON, [this, key##k##Button] (wxEvent&) { \
        key##k##Button->SetLabel("Waiting for key..."); \
        key##k##Button->Bind(wxEVT_CHAR_HOOK, [this, key##k##Button] (wxKeyEvent& evt) { \
            key##k##Button->Bind(wxEVT_CHAR_HOOK, [] (wxKeyEvent&) {}); \
            this->m_key##k = evt.GetKeyCode(); \
            key##k##Button->SetLabel(wxAcceleratorEntry(0, this->m_key##k).ToString()); \
        }); \
    }); \
    key##k##Sizer->Add(key##k##Button, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));

SettingsFrame::SettingsFrame(MainFrame* parent) :
    wxFrame(parent, wxID_ANY, "Settings", wxDefaultPosition, wxSize(500, 450)),
    m_mainFrame(parent),
    m_keyUp(Config::keyUp),
    m_keyRight(Config::keyRight),
    m_keyDown(Config::keyDown),
    m_keyLeft(Config::keyLeft),
    m_key1(Config::key1),
    m_key2(Config::key2),
    m_key12(Config::key12)
{
    wxPanel* framePanel = new wxPanel(this);
    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    framePanel->SetSizer(frameSizer);

    wxAuiNotebook* notebook = new wxAuiNotebook(framePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE);
    frameSizer->Add(notebook, wxSizerFlags().Proportion(1).Expand().Border());


    // General page
    wxPanel* generalPage = new wxPanel(notebook);
    notebook->AddPage(generalPage, "General");
    wxBoxSizer* generalSizer = new wxBoxSizer(wxVERTICAL);
    generalPage->SetSizer(generalSizer);

    wxHyperlinkCtrl* helperLink = new wxHyperlinkCtrl(generalPage, wxID_ANY, "See the MANUAL for more information", "https://github.com/Stovent/CeDImu/blob/master/MANUAL.md");
    generalSizer->Add(helperLink, wxSizerFlags().Border());

    // Disc path
    wxStaticBoxSizer* discSizer = new wxStaticBoxSizer(wxVERTICAL, generalPage, "Disc");
    generalSizer->Add(discSizer, wxSizerFlags().Expand());

    wxBoxSizer* discPathRow = new wxBoxSizer(wxHORIZONTAL);
    wxTextCtrl* discPath = new wxTextCtrl(generalPage, wxID_ANY, Config::discDirectory);
    wxButton* discSelect = new wxButton(generalPage, wxID_ANY, "Select Discs directory", wxDefaultPosition, wxSize(125, 0));
    discSelect->Bind(wxEVT_BUTTON, [this, discPath] (wxEvent&) {
        wxDirDialog dirDlg(this, "Select disc directory", discPath->GetValue().ToStdString(), wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        if(dirDlg.ShowModal() == wxID_OK)
            discPath->SetValue(dirDlg.GetPath());
    });
    discPathRow->Add(discPath, wxSizerFlags().Proportion(1));
    discPathRow->Add(discSelect, wxSizerFlags().Expand());
    discSizer->Add(discPathRow, wxSizerFlags().Expand().Border());

    // BIOS / Board
    wxStaticBoxSizer* boardSizer = new wxStaticBoxSizer(wxVERTICAL, generalPage, "BIOS / Board");
    generalSizer->Add(boardSizer, wxSizerFlags().Expand());

    wxStaticText* helperTextReload = new wxStaticText(generalPage, wxID_ANY, "After changing the BIOS / Board configuration, click\n\"Emulation -> Reload core\" to apply the new configuration.");
    boardSizer->Add(helperTextReload, wxSizerFlags().Border());

    wxBoxSizer* biosPathRow = new wxBoxSizer(wxHORIZONTAL);
    wxTextCtrl* biosPath = new wxTextCtrl(generalPage, wxID_ANY, Config::systemBIOS);
    wxButton* biosSelect = new wxButton(generalPage, wxID_ANY, "Select system BIOS", wxDefaultPosition, wxSize(125, 0));
    biosSelect->Bind(wxEVT_BUTTON, [this, biosPath] (wxEvent&) {
        std::filesystem::path biosDir = biosPath->GetValue().ToStdString();
        wxFileDialog fileDlg(this, "Select system BIOS", biosDir.parent_path().string(), "", "Binary files (*.bin;*.rom)|*.bin;*.rom|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if(fileDlg.ShowModal() == wxID_OK)
            biosPath->SetValue(fileDlg.GetPath());
    });
    biosPathRow->Add(biosPath, wxSizerFlags().Proportion(1).Expand());
    biosPathRow->Add(biosSelect, wxSizerFlags().Expand());
    boardSizer->Add(biosPathRow, wxSizerFlags().Expand().Border());

    wxBoxSizer* boardTypeRow = new wxBoxSizer(wxHORIZONTAL);
    boardSizer->Add(boardTypeRow, wxSizerFlags().Expand().Border());
    wxStaticText* boardLabel = new wxStaticText(generalPage, wxID_ANY, "Board : ");
    boardTypeRow->Add(boardLabel);

    wxRadioButton* autoType = new wxRadioButton(generalPage, wxID_ANY, "Auto");
    autoType->SetValue(Config::boardType == Boards::AutoDetect);
    boardTypeRow->Add(autoType);
    wxRadioButton* mono34RobocoType = new wxRadioButton(generalPage, wxID_ANY, "Mono 3/4/Roboco");
    mono34RobocoType->SetValue(Config::boardType == Boards::Mono3 || Config::boardType == Boards::Mono4 || Config::boardType == Boards::Roboco);
    boardTypeRow->Add(mono34RobocoType);

    wxCheckBox* has32KbNvram = new wxCheckBox(generalPage, wxID_ANY, "32KB NVRAM");
    has32KbNvram->SetValue(Config::has32KBNVRAM);
    boardSizer->Add(has32KbNvram, wxSizerFlags().Border());

    wxCheckBox* palCheckBox = new wxCheckBox(generalPage, wxID_ANY, "PAL");
    palCheckBox->SetValue(Config::PAL);
    boardSizer->Add(palCheckBox, wxSizerFlags().Border());

    wxBoxSizer* initialTimeRow = new wxBoxSizer(wxHORIZONTAL);
    boardSizer->Add(initialTimeRow, wxSizerFlags().Border());
    wxTextCtrl* initialTimeText = new wxTextCtrl(generalPage, wxID_ANY, Config::initialTime);
    initialTimeRow->Add(initialTimeText);
    wxStaticText* initialTimeHelperText = new wxStaticText(generalPage, wxID_ANY, "Initial time (in UNIX timestamp format)");
    initialTimeRow->Add(initialTimeHelperText);
    wxStaticText* initialTimeHelperText2 = new wxStaticText(generalPage, wxID_ANY, "Leave empty to use the current time.");
    boardSizer->Add(initialTimeHelperText2);
    wxStaticText* initialTimeHelperText3 = new wxStaticText(generalPage, wxID_ANY, "0 to use the previously saved time in the nvram.");
    boardSizer->Add(initialTimeHelperText3);



    // Controls page
    wxPanel* controlsPanel = new wxPanel(notebook);
    notebook->AddPage(controlsPanel, "Controls");
    wxStaticBoxSizer* controlsSizer = new wxStaticBoxSizer(wxVERTICAL, controlsPanel, "Gamepad");
    controlsPanel->SetSizer(controlsSizer);

    CREATE_CONTROL_BUTTON(Up, "Up")
    CREATE_CONTROL_BUTTON(Right, "Right")
    CREATE_CONTROL_BUTTON(Down, "Down")
    CREATE_CONTROL_BUTTON(Left, "Left")
    CREATE_CONTROL_BUTTON(1, "Button 1")
    CREATE_CONTROL_BUTTON(2, "Button 2")
    CREATE_CONTROL_BUTTON(12, "Button 1+2")



    // Bot buttons
    wxPanel* buttonsPanel = new wxPanel(framePanel);
    frameSizer->Add(buttonsPanel, wxSizerFlags().Right());
    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonsPanel->SetSizer(buttonsSizer);

    wxButton* saveButon = new wxButton(buttonsPanel, wxID_ANY, "Save");
    saveButon->Bind(wxEVT_BUTTON, [this, biosPath, discPath, autoType, mono34RobocoType, has32KbNvram, palCheckBox, initialTimeText] (wxEvent&) {
        Config::systemBIOS = biosPath->GetValue();
        Config::discDirectory = discPath->GetValue();
        Config::PAL = palCheckBox->GetValue();
        Config::initialTime = initialTimeText->GetValue();
        Config::has32KBNVRAM = has32KbNvram->GetValue();
        if(mono34RobocoType->GetValue())
            Config::boardType = Boards::Mono3;
        else
            Config::boardType = Boards::AutoDetect;

        Config::keyUp = this->m_keyUp;
        Config::keyRight = this->m_keyRight;
        Config::keyDown = this->m_keyDown;
        Config::keyLeft = this->m_keyLeft;
        Config::key1 = this->m_key1;
        Config::key2 = this->m_key2;
        Config::key12 = this->m_key12;

        if(Config::saveConfig())
            this->Close();
        else
            wxMessageBox("Failed to save config to file");
    });
    buttonsSizer->Add(saveButon, wxSizerFlags().Border());

    wxButton* cancelButton = new wxButton(buttonsPanel, wxID_ANY, "Cancel");
    cancelButton->Bind(wxEVT_BUTTON, [this] (wxEvent&) {
        this->Close();
    });
    buttonsSizer->Add(cancelButton, wxSizerFlags().Border());

    Layout();
    Show();
}

SettingsFrame::~SettingsFrame()
{
    m_mainFrame->m_settingsFrame = nullptr;
}
