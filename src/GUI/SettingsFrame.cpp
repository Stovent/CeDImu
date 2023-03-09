#include "SettingsFrame.hpp"
#include "enums.hpp"
#include "MainFrame.hpp"

#include <wx/arrstr.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/hyperlink.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textdlg.h>
#include <wx/aui/auibook.h>

#include <filesystem>

void createControlButton(wxPanel* controlsPanel, int* key, const wxString& label, wxBoxSizer* nameSizer, wxBoxSizer* buttonSizer)
{
    wxStaticText* keyText = new wxStaticText(controlsPanel, wxID_ANY, label);
    wxButton* keyButton = new wxButton(controlsPanel, wxID_ANY, *key ? wxAcceleratorEntry(0, *key).ToString() : wxString(), wxDefaultPosition, wxSize(100, 25));
    keyButton->Bind(wxEVT_BUTTON, [key, keyButton] (wxEvent&) {
        keyButton->Bind(wxEVT_CHAR_HOOK, [key, keyButton] (wxKeyEvent& evt) {
            keyButton->Bind(wxEVT_CHAR_HOOK, [] (wxKeyEvent&) {});
            *key = evt.GetKeyCode();
            keyButton->SetLabel(*key ? wxAcceleratorEntry(0, *key).ToString() : wxString());
        });
        keyButton->SetLabel("Waiting for key...");
    });

    nameSizer->Add(keyText, wxSizerFlags().DoubleBorder().Right());
    buttonSizer->Add(keyButton, wxSizerFlags().Border().Expand());
}

wxBEGIN_EVENT_TABLE(SettingsFrame, wxFrame)
    EVT_BUTTON(IDSettingsFrameOnNewConfig, SettingsFrame::OnNewConfig)
    EVT_BUTTON(IDSettingsFrameOnDeleteConfig, SettingsFrame::OnDeleteConfig)
    EVT_LISTBOX(IDSettingsFrameOnSelectBios, SettingsFrame::OnSelectBios)
wxEND_EVENT_TABLE()

SettingsFrame::SettingsFrame(MainFrame* parent)
    : wxFrame(parent, wxID_ANY, "Settings")
    , m_mainFrame(parent)
    , m_biosConfigs(Config::bioses)
    , m_lastSelection(0)
    , m_keyUp(Config::keyUp)
    , m_keyRight(Config::keyRight)
    , m_keyDown(Config::keyDown)
    , m_keyLeft(Config::keyLeft)
    , m_key1(Config::key1)
    , m_key2(Config::key2)
    , m_key12(Config::key12)
{
    SetSizeHints(wxSize(500, 500)); // For some reason on wxGTK (at least in WSL2), the window has a client size of (0, 0) without a default size.
    wxPanel* framePanel = new wxPanel(this);
    wxAuiNotebook* notebook = new wxAuiNotebook(framePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE);

    // General page
    wxPanel* generalPage = new wxPanel(notebook);

    wxHyperlinkCtrl* helperLink = new wxHyperlinkCtrl(generalPage, wxID_ANY, "See the MANUAL for more information", "https://github.com/Stovent/CeDImu/blob/master/MANUAL.md");

    // Disc path
    m_discPath = new wxTextCtrl(generalPage, wxID_ANY, Config::discDirectory);
    wxButton* discSelect = new wxButton(generalPage, wxID_ANY, "Select Discs directory", wxDefaultPosition, wxSize(125, 0));
    discSelect->Bind(wxEVT_BUTTON, [this] (wxEvent&) {
        wxDirDialog dirDlg(this, "Select disc directory", this->m_discPath->GetValue().ToStdString(), wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        if(dirDlg.ShowModal() == wxID_OK)
            this->m_discPath->SetValue(dirDlg.GetPath());
    });
    wxBoxSizer* discPathRow = new wxBoxSizer(wxHORIZONTAL);
    discPathRow->Add(m_discPath, wxSizerFlags(1));
    discPathRow->Add(discSelect, wxSizerFlags().Expand());

    wxStaticBoxSizer* discSizer = new wxStaticBoxSizer(wxVERTICAL, generalPage, "Disc");
    discSizer->Add(discPathRow, wxSizerFlags().Expand().Border());


    // BIOSes
    wxStaticText* helperTextReload = new wxStaticText(generalPage, wxID_ANY, "After changing a BIOS configuration, click\n\"Emulation -> Reload core\" to apply the new configuration.");

    // BIOS Buttons
    wxButton* biosNewButton = new wxButton(generalPage, IDSettingsFrameOnNewConfig, "New");
    wxButton* biosDeleteButton = new wxButton(generalPage, IDSettingsFrameOnDeleteConfig, "Delete");
    wxBoxSizer* biosListButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    biosListButtonSizer->Add(biosNewButton);
    biosListButtonSizer->Add(biosDeleteButton);

    // BIOS list box
    wxArrayString biosStrings;
    for(size_t i = 0; i < m_biosConfigs.size(); i++)
        biosStrings.Add(wxString(m_biosConfigs[i].name));

    m_biosList = new wxListBox(generalPage, IDSettingsFrameOnSelectBios, wxDefaultPosition, wxDefaultSize, biosStrings);

    wxBoxSizer* biosListSizer = new wxBoxSizer(wxVERTICAL);
    biosListSizer->Add(biosListButtonSizer, wxSizerFlags().Border().Expand());
    biosListSizer->Add(m_biosList, wxSizerFlags(1).Border().Expand());

    // BIOS config
    wxPanel* biosConfigPage = new wxPanel(generalPage);

    // BIOS path
    m_biosPath = new wxTextCtrl(biosConfigPage, wxID_ANY);
    m_biosSelect = new wxButton(biosConfigPage, wxID_ANY, "Select BIOS file", wxDefaultPosition, wxSize(115, 0));
    m_biosSelect->Bind(wxEVT_BUTTON, [this] (wxEvent&) {
        std::filesystem::path biosDir = this->m_biosPath->GetValue().ToStdString();
        wxFileDialog fileDlg(this, "Select system BIOS", biosDir.parent_path().string(), "", "Binary files (*.bin;*.rom)|*.bin;*.rom|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if(fileDlg.ShowModal() == wxID_OK)
        {
            const wxString path = fileDlg.GetPath();
            this->m_biosPath->SetValue(path);
            if(this->m_nvramFileName->GetValue().size() == 0)
                this->m_nvramFileName->SetValue("nvram_" + std::filesystem::path(path.ToStdString()).stem().string() + ".bin");
        }
    });
    wxBoxSizer* biosPathSizer = new wxBoxSizer(wxHORIZONTAL);
    biosPathSizer->Add(m_biosPath, wxSizerFlags(1).Expand());
    biosPathSizer->Add(m_biosSelect, wxSizerFlags().Expand());

    // NVRAM file name
    m_nvramFileName = new wxTextCtrl(biosConfigPage, wxID_ANY);
    wxStaticText* nvramFileNameText = new wxStaticText(biosConfigPage, wxID_ANY, " Associated NVRAM filename");
    wxBoxSizer* nvramFileNameSizer = new wxBoxSizer(wxHORIZONTAL);
    nvramFileNameSizer->Add(m_nvramFileName, wxSizerFlags().Proportion(1).Expand());
    nvramFileNameSizer->Add(nvramFileNameText, wxSizerFlags().Expand());

    // Board type
    constexpr int boardChoicesLength = static_cast<int>(Boards::Fail); // Fail is the last item in the enum, but not a selectable option.
    wxString boardsChoices[boardChoicesLength];
    for(size_t i = 0; i < boardChoicesLength; i++)
        boardsChoices[i] = wxString(BoardsToString(static_cast<Boards>(i)));

    m_boardChoice = new wxChoice(biosConfigPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, boardChoicesLength, boardsChoices);
    wxStaticText* boardChoiceText = new wxStaticText(biosConfigPage, wxID_ANY, " Board");
    wxBoxSizer* boardChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
    boardChoiceSizer->Add(m_boardChoice, wxSizerFlags(1).Expand());
    boardChoiceSizer->Add(boardChoiceText);

    // PAL and NVRAM check boxes
    m_palCheckBox = new wxCheckBox(biosConfigPage, wxID_ANY, "PAL");
    m_nvramCheckBox = new wxCheckBox(biosConfigPage, wxID_ANY, "32KB NVRAM");
    wxBoxSizer* checkBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    checkBoxSizer->Add(m_palCheckBox, wxSizerFlags(1).Border());
    checkBoxSizer->Add(m_nvramCheckBox, wxSizerFlags(1).Border());

    // Initial timestamp
    m_initialTime = new wxTextCtrl(biosConfigPage, wxID_ANY);
    wxStaticText* initialTimeHelperText = new wxStaticText(biosConfigPage, wxID_ANY, " Initial time (in UNIX timestamp format)");
    wxStaticText* initialTimeHelperText2 = new wxStaticText(biosConfigPage, wxID_ANY, "Leave empty to use the current time.");
    wxStaticText* initialTimeHelperText3 = new wxStaticText(biosConfigPage, wxID_ANY, "0 to use the previously saved time in the nvram.");
    wxStaticText* initialTimeHelperText4 = new wxStaticText(biosConfigPage, wxID_ANY, "Default is 599616000 (1989/01/01 00:00:00).");
    wxBoxSizer* initialTimeSizer = new wxBoxSizer(wxHORIZONTAL);
    initialTimeSizer->Add(m_initialTime);
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

    createControlButton(controlsPanel, &m_keyUp, "Up", nameLeftSizer, buttonsLeftSizer);
    createControlButton(controlsPanel, &m_keyRight, "Right", nameLeftSizer,buttonsLeftSizer );
    createControlButton(controlsPanel, &m_keyDown, "Down", nameLeftSizer, buttonsLeftSizer);
    createControlButton(controlsPanel, &m_keyLeft, "Left", nameLeftSizer, buttonsLeftSizer);
    createControlButton(controlsPanel, &m_key1, "Button 1", nameRightSizer, buttonsRightSizer);
    createControlButton(controlsPanel, &m_key2, "Button 2", nameRightSizer, buttonsRightSizer);
    createControlButton(controlsPanel, &m_key12, "Button 1+2", nameRightSizer, buttonsRightSizer);

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
    saveButon->Bind(wxEVT_BUTTON, &SettingsFrame::OnSaveConfig, this);

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

    if(m_biosConfigs.size() > 0)
        m_biosList->SetSelection(m_lastSelection);

    LoadSelection();

    Fit();
    Layout();
    Show();
}

SettingsFrame::~SettingsFrame()
{
    m_mainFrame->m_settingsFrame = nullptr;
}

void SettingsFrame::OnSaveConfig(wxEvent&)
{
    SaveSelection();

    Config::discDirectory = m_discPath->GetValue();

    Config::keyUp = m_keyUp;
    Config::keyRight = m_keyRight;
    Config::keyDown = m_keyDown;
    Config::keyLeft = m_keyLeft;
    Config::key1 = m_key1;
    Config::key2 = m_key2;
    Config::key12 = m_key12;

    Config::bioses = m_biosConfigs;

    if(Config::saveConfig())
    {
        m_mainFrame->CreateBiosMenu();
        Close();
    }
    else
        wxMessageBox("Failed to save config to file");
}

void SettingsFrame::OnNewConfig(wxCommandEvent&)
{
    wxTextEntryDialog dlg(this, "Enter the name of the new BIOS configuration:", "New BIOS configuration");

    if(dlg.ShowModal() == wxID_OK)
    {
        const wxString name = dlg.GetValue().Trim(true).Trim(false);
        if(name.IsEmpty())
            return;

        SaveSelection();
        m_lastSelection = m_biosConfigs.size();
        Config::BiosConfig& config = m_biosConfigs.emplace_back(Config::defaultBiosConfig);
        config.name = name;

        m_biosList->InsertItems(1, &name, m_lastSelection);
        m_biosList->SetSelection(m_lastSelection);
        m_biosList->EnsureVisible(m_lastSelection);

        LoadSelection();
    }
}

void SettingsFrame::OnDeleteConfig(wxCommandEvent&)
{
    if(static_cast<size_t>(m_lastSelection) >= m_biosConfigs.size())
        return;

    m_biosConfigs.erase(std::next(m_biosConfigs.cbegin(), m_lastSelection));
    m_biosList->Delete(m_lastSelection);

    if(static_cast<size_t>(m_lastSelection) >= m_biosConfigs.size())
        m_lastSelection--;

    if(static_cast<size_t>(m_lastSelection) < m_biosConfigs.size())
    {
        m_biosList->SetSelection(m_lastSelection);
        m_biosList->EnsureVisible(m_lastSelection);
    }

    LoadSelection();
}

void SettingsFrame::OnSelectBios(wxCommandEvent& event)
{
    SaveSelection();
    m_lastSelection = event.GetSelection();
    LoadSelection();
}

void SettingsFrame::LoadSelection()
{
    CheckControls();

    if(static_cast<size_t>(m_lastSelection) >= m_biosConfigs.size())
        return;

    const Config::BiosConfig& config = m_biosConfigs[m_lastSelection];

    m_biosPath->SetValue(config.biosFilePath);
    m_nvramFileName->SetValue(config.nvramFileName);
    m_initialTime->SetValue(config.initialTime);
    m_boardChoice->SetSelection(static_cast<int>(config.boardType));
    m_palCheckBox->SetValue(config.PAL);
    m_nvramCheckBox->SetValue(config.has32KbNvram);
}

void SettingsFrame::SaveSelection()
{
    if(static_cast<size_t>(m_lastSelection) >= m_biosConfigs.size())
        return;

    Config::BiosConfig& config = m_biosConfigs[m_lastSelection];

    config.biosFilePath = m_biosPath->GetValue();
    config.nvramFileName = m_nvramFileName->GetValue();
    config.initialTime = m_initialTime->GetValue();
    config.boardType = static_cast<Boards>(m_boardChoice->GetSelection());
    config.PAL = m_palCheckBox->GetValue();
    config.has32KbNvram = m_nvramCheckBox->GetValue();
}

void SettingsFrame::CheckControls()
{
    const bool enable = m_biosConfigs.size() != 0 ? true : false;

    m_biosPath->Enable(enable);
    m_biosSelect->Enable(enable);
    m_nvramFileName->Enable(enable);
    m_initialTime->Enable(enable);
    m_boardChoice->Enable(enable);
    m_palCheckBox->Enable(enable);
    m_nvramCheckBox->Enable(enable);
}
