#include "SettingsFrame.hpp"
#include "MainFrame.hpp"
#include "../Config.hpp"

#include <wx/button.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/notebook.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

SettingsFrame::SettingsFrame(MainFrame* parent) :
    wxFrame(parent, wxID_ANY, "Settings", wxDefaultPosition, wxSize(450, 280)),
    mainFrame(parent)
{
#ifdef _WIN32
    char separator = '\\';
#else
    char separator = '/';
#endif // _WIN32
    wxPanel* settingsPanel = new wxPanel(this);
    wxNotebook* notebook = new wxNotebook(settingsPanel, wxID_ANY);

    // General page
    wxPanel* generalPage = new wxPanel(notebook);
    wxSizer* generalSizer = new wxBoxSizer(wxVERTICAL);
    wxSizer* generalSizerGeneral = new wxBoxSizer(wxHORIZONTAL);
    wxSizer* generalSizerEmulation = new wxBoxSizer(wxHORIZONTAL);

    // BIOS and ROM
    wxSizer* generalStaticSizer = new wxStaticBoxSizer(wxVERTICAL, generalPage, "BIOS / ROM");
    wxSizer* generalRowBiosType = new wxBoxSizer(wxHORIZONTAL);
    wxSizer* generalRowBios = new wxBoxSizer(wxHORIZONTAL);
    wxSizer* generalRowRom = new wxBoxSizer(wxHORIZONTAL);

    wxRadioButton* radioAuto = new wxRadioButton(generalPage, wxID_ANY, "Auto detect");
    radioAuto->SetValue(Config::boardType == Boards::AutoDetect);
    wxRadioButton* radioMiniMMC = new wxRadioButton(generalPage, wxID_ANY, "Mini-MMC");
    radioMiniMMC->SetValue(Config::boardType == Boards::MiniMMC);
    wxRadioButton* radioMono3 = new wxRadioButton(generalPage, wxID_ANY, "Mono-3");
    radioMono3->SetValue(Config::boardType == Boards::Mono3);
    wxRadioButton* radioMono4 = new wxRadioButton(generalPage, wxID_ANY, "Mono-4");
    radioMono4->SetValue(Config::boardType == Boards::Mono4);
    wxCheckBox* checkHas32KBNVRAM = new wxCheckBox(generalPage, wxID_ANY, "32KB NVRAM");
    checkHas32KBNVRAM->SetValue(Config::has32KBNVRAM);

    wxTextCtrl* biosPath = new wxTextCtrl(generalPage, wxID_ANY, Config::systemBIOS);
    wxButton* biosButton = new wxButton(generalPage, wxID_ANY, "Select system BIOS");
    biosButton->Bind(wxEVT_BUTTON, [this, biosPath, separator] (wxEvent&) {
        wxFileDialog openFileDialog(this, "Load system BIOS", wxString(Config::systemBIOS).BeforeLast(separator), "", "All files (*.*)|*.*|Binary files (*.bin,*.rom)|*.bin,*.rom", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if(openFileDialog.ShowModal() == wxID_OK)
            biosPath->SetValue(openFileDialog.GetPath());
    });
    wxTextCtrl* romPath = new wxTextCtrl(generalPage, wxID_ANY, Config::ROMDirectory);
    wxButton* romButton = new wxButton(generalPage, wxID_ANY, "Select ROMs directory");
    romButton->Bind(wxEVT_BUTTON, [this, romPath] (wxEvent&) {
        wxDirDialog dirDialog(this, wxDirSelectorPromptStr, Config::ROMDirectory, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        if(dirDialog.ShowModal() == wxID_OK)
            romPath->SetValue(dirDialog.GetPath());
    });

    generalRowBiosType->Add(radioAuto);
    generalRowBiosType->Add(radioMiniMMC);
    generalRowBiosType->Add(radioMono3);
    generalRowBiosType->Add(radioMono4);
    generalRowBios->Add(biosPath, wxSizerFlags().Proportion(1));
    generalRowBios->Add(biosButton);
    generalRowRom->Add(romPath, wxSizerFlags().Proportion(1));
    generalRowRom->Add(romButton);
    generalStaticSizer->Add(generalRowBiosType, wxSizerFlags().Expand());
    generalStaticSizer->Add(checkHas32KBNVRAM, wxSizerFlags().Expand());
    generalStaticSizer->Add(generalRowBios, wxSizerFlags().Expand());
    generalStaticSizer->Add(generalRowRom, wxSizerFlags().Expand());

    // Emulation
    wxSizer* emulationStaticSizer = new wxStaticBoxSizer(wxVERTICAL, generalPage, "Emulation");
    wxCheckBox* palCheckBox = new wxCheckBox(generalPage, wxID_ANY, "PAL");
    palCheckBox->SetValue(Config::PAL);
    wxTextCtrl* initialTime = new wxTextCtrl(generalPage, wxID_ANY);
    initialTime->SetValue(Config::initialTime);
    wxStaticText* initialTimeLabel = new wxStaticText(generalPage, wxID_ANY, "Initial time (Unix timestamp)");
    wxStaticText* explainationText = new wxStaticText(generalPage, wxID_ANY, "empty for current time, 0 for previous time, non-0 for any time.");

    wxSizer* initialTimeRowSizer = new wxBoxSizer(wxHORIZONTAL);
    initialTimeRowSizer->Add(initialTime);
    initialTimeRowSizer->Add(initialTimeLabel);

    emulationStaticSizer->Add(palCheckBox);
    emulationStaticSizer->Add(initialTimeRowSizer);
    emulationStaticSizer->Add(explainationText);


    generalSizerGeneral->Add(generalStaticSizer, wxSizerFlags().Proportion(1));
    generalSizerEmulation->Add(emulationStaticSizer, wxSizerFlags().Proportion(1));
    generalSizer->Add(generalSizerGeneral, wxSizerFlags().Expand());
    generalSizer->Add(generalSizerEmulation, wxSizerFlags().Expand());
    generalPage->SetSizer(generalSizer);

    notebook->AddPage(generalPage, "General");

    // TODO: Controls page


    wxPanel* buttonsPanel = new wxPanel(settingsPanel);
    wxButton* saveButton = new wxButton(buttonsPanel, wxID_ANY, "Save");
    saveButton->Bind(wxEVT_BUTTON, [this, radioMiniMMC, radioMono3, radioMono4, radioAuto, checkHas32KBNVRAM, biosPath, romPath, palCheckBox, initialTime] (wxEvent&) {
        if(radioMiniMMC->GetValue()) Config::boardType = Boards::MiniMMC;
        else if(radioMono3->GetValue()) Config::boardType = Boards::Mono3;
        else if(radioMono4->GetValue()) Config::boardType = Boards::Mono4;
        else Config::boardType = Boards::AutoDetect;
        Config::has32KBNVRAM = checkHas32KBNVRAM->GetValue();
        Config::systemBIOS = biosPath->GetValue();
        Config::ROMDirectory = romPath->GetValue();
        Config::PAL = palCheckBox->GetValue();
        Config::initialTime = initialTime->GetValue();
        if(Config::saveConfig())
            this->Close();
        else
            wxMessageBox("Failed to save config");
    });
    wxButton* cancelButton = new wxButton(buttonsPanel, wxID_ANY, "Cancel");
    cancelButton->Bind(wxEVT_BUTTON, [this] (wxEvent&) {
        this->Close();
    });
    wxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonsSizer->Add(saveButton);
    buttonsSizer->Add(cancelButton);
    buttonsPanel->SetSizer(buttonsSizer);

    wxSizer* settingsPanelSizer = new wxBoxSizer(wxVERTICAL);
    settingsPanelSizer->Add(notebook, wxSizerFlags().Proportion(1).Expand());
    settingsPanelSizer->Add(buttonsPanel, wxSizerFlags().Align(wxALIGN_RIGHT));
    settingsPanel->SetSizer(settingsPanelSizer);
}

SettingsFrame::~SettingsFrame()
{
    mainFrame->settingsFrame = nullptr;
}
