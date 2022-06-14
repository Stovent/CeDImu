#include "SettingsFrame.hpp"
#include "MainFrame.hpp"
#include "../Config.hpp"

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/hyperlink.h>
#include <wx/listbox.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
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
    : wxFrame(parent, wxID_ANY, "Settings")
    , m_mainFrame(parent)
    , m_keyUp(Config::keyUp)
    , m_keyRight(Config::keyRight)
    , m_keyDown(Config::keyDown)
    , m_keyLeft(Config::keyLeft)
    , m_key1(Config::key1)
    , m_key2(Config::key2)
    , m_key12(Config::key12)
{
    SetSizeHints(wxSize(500, 500)); // For some reason on wxGTK, the window has a client size of 0, 0 without this.
    wxPanel* framePanel = new wxPanel(this);
    wxAuiNotebook* notebook = new wxAuiNotebook(framePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE);

    // General page
    wxPanel* generalPage = new wxPanel(notebook);

    wxHyperlinkCtrl* helperLink = new wxHyperlinkCtrl(generalPage, wxID_ANY, "See the MANUAL for more information", "https://github.com/Stovent/CeDImu/blob/master/MANUAL.md");

    // Disc path
    wxTextCtrl* discPath = new wxTextCtrl(generalPage, wxID_ANY, Config::discDirectory);
    wxButton* discSelect = new wxButton(generalPage, wxID_ANY, "Select Discs directory", wxDefaultPosition, wxSize(125, 0));
    discSelect->Bind(wxEVT_BUTTON, [this, discPath] (wxEvent&) {
        wxDirDialog dirDlg(this, "Select disc directory", discPath->GetValue().ToStdString(), wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        if(dirDlg.ShowModal() == wxID_OK)
            discPath->SetValue(dirDlg.GetPath());
    });
    wxBoxSizer* discPathRow = new wxBoxSizer(wxHORIZONTAL);
    discPathRow->Add(discPath, wxSizerFlags(1));
    discPathRow->Add(discSelect, wxSizerFlags().Expand());

    wxStaticBoxSizer* discSizer = new wxStaticBoxSizer(wxVERTICAL, generalPage, "Disc");
    discSizer->Add(discPathRow, wxSizerFlags().Expand().Border());


    // BIOSes
    wxStaticText* helperTextReload = new wxStaticText(generalPage, wxID_ANY, "After changing a BIOS configuration, click\n\"Emulation -> Reload core\" to apply the new configuration.");

    // BIOS Buttons
    wxButton* biosNewButton = new wxButton(generalPage, wxID_ANY, "New");
    wxButton* biosDeleteButton = new wxButton(generalPage, wxID_ANY, "Delete");
    wxBoxSizer* biosListButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    biosListButtonSizer->Add(biosNewButton);
    biosListButtonSizer->Add(biosDeleteButton);

    // BIOS list box
    wxString biosStrings[Config::bioses.size()];
    for(size_t i = 0; i < Config::bioses.size(); i++)
        biosStrings[i] = wxString(Config::bioses[i].name);

    wxListBox* biosList = new wxListBox(generalPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, Config::bioses.size(), biosStrings);

    wxBoxSizer* biosListSizer = new wxBoxSizer(wxVERTICAL);
    biosListSizer->Add(biosListButtonSizer, wxSizerFlags().Border().Expand());
    biosListSizer->Add(biosList, wxSizerFlags(1).Border().Expand());

    // BIOS config
    wxPanel* biosConfigPage = new wxPanel(generalPage);

    // BIOS path
    wxTextCtrl* biosPath = new wxTextCtrl(biosConfigPage, wxID_ANY);
    wxButton* biosSelect = new wxButton(biosConfigPage, wxID_ANY, "Select BIOS file", wxDefaultPosition, wxSize(115, 0));
    biosSelect->Bind(wxEVT_BUTTON, [this, biosPath] (wxEvent&) {
        std::filesystem::path biosDir = biosPath->GetValue().ToStdString();
        wxFileDialog fileDlg(this, "Select system BIOS", biosDir.parent_path().string(), "", "Binary files (*.bin;*.rom)|*.bin;*.rom|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if(fileDlg.ShowModal() == wxID_OK)
            biosPath->SetValue(fileDlg.GetPath());
    });
    wxBoxSizer* biosPathSizer = new wxBoxSizer(wxHORIZONTAL);
    biosPathSizer->Add(biosPath, wxSizerFlags(1).Expand());
    biosPathSizer->Add(biosSelect, wxSizerFlags().Expand());

    // NVRAM file name
    wxTextCtrl* nvramFileName = new wxTextCtrl(biosConfigPage, wxID_ANY);
    wxStaticText* nvramFileNameText = new wxStaticText(biosConfigPage, wxID_ANY, " Associated NVRAM filename");
    wxBoxSizer* nvramFileNameSizer = new wxBoxSizer(wxHORIZONTAL);
    nvramFileNameSizer->Add(nvramFileName, wxSizerFlags().Proportion(1).Expand());
    nvramFileNameSizer->Add(nvramFileNameText, wxSizerFlags().Expand());

    // Board type
    constexpr int boardChoicesLength = static_cast<int>(Boards::Fail); // Fail is the last item in the enum, but not a selectable option.
    wxString boardsChoices[boardChoicesLength];
    for(size_t i = 0; i < boardChoicesLength; i++)
        boardsChoices[i] = wxString(BoardsToString(static_cast<Boards>(i)));

    wxChoice* boardChoice = new wxChoice(biosConfigPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, boardChoicesLength, boardsChoices);
    wxStaticText* boardChoiceText = new wxStaticText(biosConfigPage, wxID_ANY, " Board");
    wxBoxSizer* boardChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
    boardChoiceSizer->Add(boardChoice, wxSizerFlags(1).Expand());
    boardChoiceSizer->Add(boardChoiceText);

    // PAL and NVRAM check boxes
    wxCheckBox* palCheckBox = new wxCheckBox(biosConfigPage, wxID_ANY, "PAL");
    wxCheckBox* nvramCheckBox = new wxCheckBox(biosConfigPage, wxID_ANY, "32KB NVRAM");
    wxBoxSizer* checkBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    checkBoxSizer->Add(palCheckBox, wxSizerFlags(1).Border());
    checkBoxSizer->Add(nvramCheckBox, wxSizerFlags(1).Border());

    // Initial timestamp
    wxTextCtrl* initialTimeText = new wxTextCtrl(biosConfigPage, wxID_ANY);
    wxStaticText* initialTimeHelperText = new wxStaticText(biosConfigPage, wxID_ANY, " Initial time (in UNIX timestamp format)");
    wxStaticText* initialTimeHelperText2 = new wxStaticText(biosConfigPage, wxID_ANY, "Leave empty to use the current time.");
    wxStaticText* initialTimeHelperText3 = new wxStaticText(biosConfigPage, wxID_ANY, "0 to use the previously saved time in the nvram.");
    wxStaticText* initialTimeHelperText4 = new wxStaticText(biosConfigPage, wxID_ANY, "Default is 599616000 (1989/01/01 00:00:00).");
    wxBoxSizer* initialTimeSizer = new wxBoxSizer(wxHORIZONTAL);
    initialTimeSizer->Add(initialTimeText);
    initialTimeSizer->Add(initialTimeHelperText);

    wxStaticBoxSizer* biosConfigPageSizer = new wxStaticBoxSizer(wxVERTICAL, biosConfigPage);
    biosConfigPageSizer->Add(biosPathSizer, wxSizerFlags().Expand());
    biosConfigPageSizer->Add(nvramFileNameSizer, wxSizerFlags().Expand().Border(wxTOP));
    biosConfigPageSizer->Add(boardChoiceSizer, wxSizerFlags().Expand().Border(wxTOP));
    biosConfigPageSizer->Add(checkBoxSizer);
    biosConfigPageSizer->Add(initialTimeSizer, wxSizerFlags().Border());
    biosConfigPageSizer->Add(initialTimeHelperText2);
    biosConfigPageSizer->Add(initialTimeHelperText3);
    biosConfigPageSizer->Add(initialTimeHelperText4);
    biosConfigPage->SetSizerAndFit(biosConfigPageSizer);
    biosConfigPage->SetMinSize(wxSize(500, -1));

    wxBoxSizer* biosConfigPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    biosConfigPanelSizer->Add(biosListSizer, wxSizerFlags().Border().Expand());
    biosConfigPanelSizer->Add(biosConfigPage, wxSizerFlags(1).Border().Expand());

    wxStaticBoxSizer* boardSizer = new wxStaticBoxSizer(wxVERTICAL, generalPage, "BIOS Configuration");
    boardSizer->Add(helperTextReload, wxSizerFlags().Border());
    boardSizer->Add(biosConfigPanelSizer, wxSizerFlags(1).Expand());

    wxBoxSizer* generalSizer = new wxBoxSizer(wxVERTICAL);
    generalSizer->Add(helperLink, wxSizerFlags().Border());
    generalSizer->Add(discSizer, wxSizerFlags().Expand());
    generalSizer->Add(boardSizer, wxSizerFlags().Expand());
    generalPage->SetSizerAndFit(generalSizer);

    notebook->AddPage(generalPage, "General");


    // Controls page
    wxPanel* controlsPanel = new wxPanel(notebook);

    wxBoxSizer* nameLeftSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* buttonsLeftSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* nameRightSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* buttonsRightSizer = new wxBoxSizer(wxVERTICAL);

    CREATE_CONTROL_BUTTON(Up, "Up", 0)
    CREATE_CONTROL_BUTTON(Right, "Right", 0)
    CREATE_CONTROL_BUTTON(Down, "Down", 0)
    CREATE_CONTROL_BUTTON(Left, "Left", 0)
    CREATE_CONTROL_BUTTON(1, "Button 1", 1)
    CREATE_CONTROL_BUTTON(2, "Button 2", 1)
    CREATE_CONTROL_BUTTON(12, "Button 1+2", 1)

    wxStaticBoxSizer* controlsSizer = new wxStaticBoxSizer(wxHORIZONTAL, controlsPanel, "Gamepad");
    controlsSizer->Add(nameLeftSizer, wxSizerFlags().Expand());
    controlsSizer->Add(buttonsLeftSizer, wxSizerFlags().Expand());
    controlsSizer->Add(nameRightSizer, wxSizerFlags().Expand());
    controlsSizer->Add(buttonsRightSizer, wxSizerFlags().Expand());
    controlsPanel->SetSizerAndFit(controlsSizer);

    notebook->AddPage(controlsPanel, "Controls");


    // Bot buttons
    wxPanel* buttonsPanel = new wxPanel(framePanel);

    wxButton* saveButon = new wxButton(buttonsPanel, wxID_ANY, "Save");
    saveButon->Bind(wxEVT_BUTTON, [this, discPath] (wxEvent&) {
        Config::discDirectory = discPath->GetValue();

        Config::bioses.clear();
        for(const BiosWidgets& bios : this->m_biosWidgets)
        {
            Boards board = Boards::AutoDetect;
            if(bios.boardMono34Roboco->GetValue())
                board = Boards::Mono3;

            Config::bioses.push_back(Config::BiosConfig {
                .name = bios.name->GetValue().ToStdString(),
                .biosFilePath = bios.biosPath->GetValue().ToStdString(),
                .nvramFileName = bios.nvramFileName->GetValue().ToStdString(),
                .initialTime = bios.initialTime->GetValue().ToStdString(),
                .boardType = board,
                .PAL = bios.pal->GetValue(),
                .has32KbNvram = bios.has32KbNvram->GetValue(),
            });
        }

        Config::keyUp = this->m_keyUp;
        Config::keyRight = this->m_keyRight;
        Config::keyDown = this->m_keyDown;
        Config::keyLeft = this->m_keyLeft;
        Config::key1 = this->m_key1;
        Config::key2 = this->m_key2;
        Config::key12 = this->m_key12;

        if(Config::saveConfig())
        {
            m_mainFrame->CreateBiosMenu();
            this->Close();
        }
        else
            wxMessageBox("Failed to save config to file");
    });

    wxButton* cancelButton = new wxButton(buttonsPanel, wxID_ANY, "Cancel");
    cancelButton->Bind(wxEVT_BUTTON, [this] (wxEvent&) {
        this->Close();
    });

    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonsSizer->Add(saveButon, wxSizerFlags().Border());
    buttonsSizer->Add(cancelButton, wxSizerFlags().Border());
    buttonsPanel->SetSizerAndFit(buttonsSizer);

    // Frame sizer
    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(notebook, wxSizerFlags(1).Expand().Border());
    frameSizer->Add(buttonsPanel, wxSizerFlags().Right());
    framePanel->SetSizerAndFit(frameSizer);

    Fit();
    Layout();
    Show();
}

SettingsFrame::~SettingsFrame()
{
    m_mainFrame->m_settingsFrame = nullptr;
}
