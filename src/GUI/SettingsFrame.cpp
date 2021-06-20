#include "SettingsFrame.hpp"
#include "MainFrame.hpp"
#include "../Config.hpp"

#include <wx/button.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/notebook.h>
#include <wx/sizer.h>

SettingsFrame::SettingsFrame(MainFrame* parent) :
    wxFrame(parent, wxID_ANY, "Settings"),
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

    // General
    wxSizer* generalStaticSizer = new wxStaticBoxSizer(wxVERTICAL, generalPage, "General");
    wxSizer* generalRow1 = new wxBoxSizer(wxHORIZONTAL);
    wxSizer* generalRow2 = new wxBoxSizer(wxHORIZONTAL);

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

    generalRow1->Add(biosPath, wxSizerFlags().Proportion(1));
    generalRow1->Add(biosButton);
    generalRow2->Add(romPath, wxSizerFlags().Proportion(1));
    generalRow2->Add(romButton);
    generalStaticSizer->Add(generalRow1, wxSizerFlags().Expand());
    generalStaticSizer->Add(generalRow2, wxSizerFlags().Expand());

    // Emulation
    wxSizer* emulationStaticSizer = new wxStaticBoxSizer(wxVERTICAL, generalPage, "Emulation");
    wxCheckBox* palCheckBox = new wxCheckBox(generalPage, wxID_ANY, "PAL");
    palCheckBox->SetValue(Config::PAL);

    emulationStaticSizer->Add(palCheckBox);


    generalSizerGeneral->Add(generalStaticSizer, wxSizerFlags().Proportion(1));
    generalSizerEmulation->Add(emulationStaticSizer, wxSizerFlags().Proportion(1));
    generalSizer->Add(generalSizerGeneral, wxSizerFlags().Expand());
    generalSizer->Add(generalSizerEmulation, wxSizerFlags().Expand());
    generalPage->SetSizer(generalSizer);

    notebook->AddPage(generalPage, "General");

    // Controls page


    wxPanel* buttonsPanel = new wxPanel(settingsPanel);
    wxButton* saveButton = new wxButton(buttonsPanel, wxID_ANY, "Save");
    saveButton->Bind(wxEVT_BUTTON, [this, biosPath, romPath, palCheckBox] (wxEvent&) {
        Config::systemBIOS = biosPath->GetValue();
        Config::ROMDirectory = romPath->GetValue();
        Config::PAL = palCheckBox->GetValue();
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
