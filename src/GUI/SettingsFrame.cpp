#include "SettingsFrame.hpp"
#include "MainFrame.hpp"
#include "../Config.hpp"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choicebk.h>
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

#define CREATE_CONTROL_BUTTON(k, label, side) { \
    wxStaticText* keyText = new wxStaticText(controlsPanel, wxID_ANY, label); \
    (side == 0 ? nameLeftSizer : nameRightSizer)->Add(keyText, wxSizerFlags().DoubleBorder().Right()); \
    wxButton* keyButton = new wxButton(controlsPanel, wxID_ANY, this->m_key##k ? wxAcceleratorEntry(0, m_key##k).ToString() : "", wxDefaultPosition, wxSize(100, 25)); \
    keyButton->Bind(wxEVT_BUTTON, [this, keyButton] (wxEvent&) { \
        keyButton->SetLabel("Waiting for key..."); \
        keyButton->Bind(wxEVT_CHAR_HOOK, [this, keyButton] (wxKeyEvent& evt) { \
            keyButton->Bind(wxEVT_CHAR_HOOK, [] (wxKeyEvent&) {}); \
            this->m_key##k = evt.GetKeyCode(); \
            keyButton->SetLabel(this->m_key##k ? wxAcceleratorEntry(0, this->m_key##k).ToString() : ""); \
        }); \
    }); \
    (side == 0 ? buttonsLeftSizer : buttonsRightSizer)->Add(keyButton, wxSizerFlags().Border().Expand()); }

SettingsFrame::SettingsFrame(MainFrame* parent)
    : wxFrame(parent, wxID_ANY, "Settings", wxDefaultPosition, wxSize(500, 650))
    , m_mainFrame(parent)
    , m_keyUp(Config::keyUp)
    , m_keyRight(Config::keyRight)
    , m_keyDown(Config::keyDown)
    , m_keyLeft(Config::keyLeft)
    , m_key1(Config::key1)
    , m_key2(Config::key2)
    , m_key12(Config::key12)
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


    // BIOSes
    wxStaticBoxSizer* boardSizer = new wxStaticBoxSizer(wxVERTICAL, generalPage, "BIOSes");
    generalSizer->Add(boardSizer, wxSizerFlags().Expand());

    wxStaticText* helperTextReload = new wxStaticText(generalPage, wxID_ANY, "After changing the BIOS / Board configuration, click\n\"Emulation -> Reload core\" to apply the new configuration.");
    boardSizer->Add(helperTextReload, wxSizerFlags().Border());


    // BIOS Config
    wxStaticBoxSizer* biosChoicePagePanelSizer = new wxStaticBoxSizer(wxVERTICAL, generalPage, "BIOS Configuration");
    boardSizer->Add(biosChoicePagePanelSizer, wxSizerFlags().Border().Expand().Proportion(1));

    wxChoicebook* choicebook = new wxChoicebook(generalPage, wxID_ANY);
    biosChoicePagePanelSizer->Add(choicebook, wxSizerFlags().Border().Expand());

    for(const Config::BiosConfig& biosEntry : Config::bioses)
    {
        wxPanel* choicePage = new wxPanel(choicebook);
        choicebook->AddPage(choicePage, biosEntry.name);
        wxBoxSizer* choicePageSizer = new wxBoxSizer(wxVERTICAL);
        choicePage->SetSizer(choicePageSizer);

        wxBoxSizer* biosNameRow = new wxBoxSizer(wxHORIZONTAL);
        choicePageSizer->Add(biosNameRow, wxSizerFlags().Expand().Border());
        wxTextCtrl* biosNameText = new wxTextCtrl(choicePage, wxID_ANY, biosEntry.name);
        biosNameRow->Add(biosNameText, wxSizerFlags().Proportion(1));
        wxStaticText* biosNameHelperText = new wxStaticText(choicePage, wxID_ANY, "Configuration name.");
        biosNameRow->Add(biosNameHelperText, wxSizerFlags().Proportion(0));

        wxBoxSizer* biosPathRow = new wxBoxSizer(wxHORIZONTAL);
        wxTextCtrl* biosPath = new wxTextCtrl(choicePage, wxID_ANY, biosEntry.filePath);
        wxButton* biosSelect = new wxButton(choicePage, wxID_ANY, "Select BIOS file", wxDefaultPosition, wxSize(115, 0));
        biosSelect->Bind(wxEVT_BUTTON, [this, biosPath] (wxEvent&) {
            std::filesystem::path biosDir = biosPath->GetValue().ToStdString();
            wxFileDialog fileDlg(this, "Select system BIOS", biosDir.parent_path().string(), "", "Binary files (*.bin;*.rom)|*.bin;*.rom|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
            if(fileDlg.ShowModal() == wxID_OK)
                biosPath->SetValue(fileDlg.GetPath());
        });
        biosPathRow->Add(biosPath, wxSizerFlags().Proportion(1).Expand());
        biosPathRow->Add(biosSelect, wxSizerFlags().Expand());
        choicePageSizer->Add(biosPathRow, wxSizerFlags().Expand().Border());

        wxBoxSizer* boardTypeRow = new wxBoxSizer(wxHORIZONTAL);
        choicePageSizer->Add(boardTypeRow, wxSizerFlags().Expand().Border());
        wxStaticText* boardLabel = new wxStaticText(choicePage, wxID_ANY, "Board : ");
        boardTypeRow->Add(boardLabel);

        wxRadioButton* autoType = new wxRadioButton(choicePage, wxID_ANY, "Auto");
        autoType->SetValue(biosEntry.boardType == Boards::AutoDetect);
        boardTypeRow->Add(autoType);
        wxRadioButton* mono34RobocoType = new wxRadioButton(choicePage, wxID_ANY, "Mono 3/4/Roboco");
        mono34RobocoType->SetValue(biosEntry.boardType == Boards::Mono3 || biosEntry.boardType == Boards::Mono4 || biosEntry.boardType == Boards::Roboco);
        boardTypeRow->Add(mono34RobocoType);

        wxCheckBox* has32KbNvram = new wxCheckBox(choicePage, wxID_ANY, "32KB NVRAM");
        has32KbNvram->SetValue(biosEntry.has32KBNVRAM);
        choicePageSizer->Add(has32KbNvram, wxSizerFlags().Border());

        wxCheckBox* palCheckBox = new wxCheckBox(choicePage, wxID_ANY, "PAL");
        palCheckBox->SetValue(biosEntry.PAL);
        choicePageSizer->Add(palCheckBox, wxSizerFlags().Border());

        wxBoxSizer* initialTimeRow = new wxBoxSizer(wxHORIZONTAL);
        choicePageSizer->Add(initialTimeRow, wxSizerFlags().Border());
        wxTextCtrl* initialTimeText = new wxTextCtrl(choicePage, wxID_ANY, biosEntry.initialTime);
        initialTimeRow->Add(initialTimeText);
        wxStaticText* initialTimeHelperText = new wxStaticText(choicePage, wxID_ANY, "Initial time (in UNIX timestamp format).");
        initialTimeRow->Add(initialTimeHelperText);
        wxStaticText* initialTimeHelperText2 = new wxStaticText(choicePage, wxID_ANY, "Leave empty to use the current time.");
        choicePageSizer->Add(initialTimeHelperText2);
        wxStaticText* initialTimeHelperText3 = new wxStaticText(choicePage, wxID_ANY, "0 to use the previously saved time in the nvram.");
        choicePageSizer->Add(initialTimeHelperText3);
        wxStaticText* initialTimeHelperText4 = new wxStaticText(choicePage, wxID_ANY, "Default is 599616000 (1989/01/01 00:00:00).");
        choicePageSizer->Add(initialTimeHelperText4);
    }



    // Controls page
    wxPanel* controlsPanel = new wxPanel(notebook);
    notebook->AddPage(controlsPanel, "Controls");
    wxStaticBoxSizer* controlsSizer = new wxStaticBoxSizer(wxHORIZONTAL, controlsPanel, "Gamepad");
    controlsPanel->SetSizer(controlsSizer);

    wxBoxSizer* nameLeftSizer = new wxBoxSizer(wxVERTICAL);
    controlsSizer->Add(nameLeftSizer, wxSizerFlags().Expand());
    wxBoxSizer* buttonsLeftSizer = new wxBoxSizer(wxVERTICAL);
    controlsSizer->Add(buttonsLeftSizer, wxSizerFlags().Expand());
    wxBoxSizer* nameRightSizer = new wxBoxSizer(wxVERTICAL);
    controlsSizer->Add(nameRightSizer, wxSizerFlags().Expand());
    wxBoxSizer* buttonsRightSizer = new wxBoxSizer(wxVERTICAL);
    controlsSizer->Add(buttonsRightSizer, wxSizerFlags().Expand());

    CREATE_CONTROL_BUTTON(Up, "Up", 0)
    CREATE_CONTROL_BUTTON(Right, "Right", 0)
    CREATE_CONTROL_BUTTON(Down, "Down", 0)
    CREATE_CONTROL_BUTTON(Left, "Left", 0)
    CREATE_CONTROL_BUTTON(1, "Button 1", 1)
    CREATE_CONTROL_BUTTON(2, "Button 2", 1)
    CREATE_CONTROL_BUTTON(12, "Button 1+2", 1)



    // Bot buttons
    wxPanel* buttonsPanel = new wxPanel(framePanel);
    frameSizer->Add(buttonsPanel, wxSizerFlags().Right());
    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonsPanel->SetSizer(buttonsSizer);

    wxButton* saveButon = new wxButton(buttonsPanel, wxID_ANY, "Save");
//    saveButon->Bind(wxEVT_BUTTON, [this, biosPath, discPath, autoType, mono34RobocoType, has32KbNvram, palCheckBox, initialTimeText] (wxEvent&) {
//        Config::systemBIOS = biosPath->GetValue();
//        Config::discDirectory = discPath->GetValue();
//        Config::PAL = palCheckBox->GetValue();
//        Config::initialTime = initialTimeText->GetValue();
//        Config::has32KBNVRAM = has32KbNvram->GetValue();
//        if(mono34RobocoType->GetValue())
//            Config::boardType = Boards::Mono3;
//        else
//            Config::boardType = Boards::AutoDetect;
//
//        Config::keyUp = this->m_keyUp;
//        Config::keyRight = this->m_keyRight;
//        Config::keyDown = this->m_keyDown;
//        Config::keyLeft = this->m_keyLeft;
//        Config::key1 = this->m_key1;
//        Config::key2 = this->m_key2;
//        Config::key12 = this->m_key12;
//
//        if(Config::saveConfig())
//            this->Close();
//        else
//            wxMessageBox("Failed to save config to file");
//    });
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
    m_mainFrame->CreateBiosMenu();
    m_mainFrame->m_settingsFrame = nullptr;
}
