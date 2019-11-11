#include <wx/msgdlg.h>
#include <wx/radiobut.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>

#include "RAMSearchFrame.hpp"

wxBEGIN_EVENT_TABLE(RAMSearchFrame, wxFrame)
    EVT_TIMER(wxID_ANY, RAMSearchFrame::RefreshLoop)
    EVT_CLOSE(RAMSearchFrame::OnClose)
wxEND_EVENT_TABLE()

RAMSearchFrame::RAMSearchFrame(VDSC* vds, MainFrame* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "RAM Search", pos, size)
{
    vdsc = vds;
    mainFrame = parent;

    ramSearchList = new RAMSearchList(this);

    wxPanel* buttonsPanel = new wxPanel(this);
    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonsPanel->SetSizer(buttonsSizer);
    wxStaticBoxSizer* compSymbol = new wxStaticBoxSizer(wxVERTICAL, buttonsPanel, "Comparison operator"); // button greater than, less than, etc.

    wxRadioButton* equal = new wxRadioButton(buttonsPanel, wxID_ANY, "Equal", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    equal->SetValue(true);
    compSymbol->Add(equal, 1, wxEXPAND);

    wxRadioButton* notEqual = new wxRadioButton(buttonsPanel, wxID_ANY, "Not equal");
    compSymbol->Add(notEqual, 1, wxEXPAND);

    wxRadioButton* greaterThan = new wxRadioButton(buttonsPanel, wxID_ANY, "Greater than");
    compSymbol->Add(greaterThan, 1, wxEXPAND);

    wxRadioButton* greaterThanOrEqual = new wxRadioButton(buttonsPanel, wxID_ANY, "Greater than or equal");
    compSymbol->Add(greaterThanOrEqual, 1, wxEXPAND);

    wxRadioButton* lessThan = new wxRadioButton(buttonsPanel, wxID_ANY, "Less than");
    compSymbol->Add(lessThan, 1, wxEXPAND);

    wxRadioButton* lessThanOrEqual = new wxRadioButton(buttonsPanel, wxID_ANY, "Less than or equal");
    compSymbol->Add(lessThanOrEqual, 1, wxEXPAND);

    buttonsSizer->Add(compSymbol, 0, wxEXPAND, 5);

    wxBoxSizer* dataSizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticBoxSizer* dataSize = new wxStaticBoxSizer(wxVERTICAL, buttonsPanel, "Data display");
    wxRadioButton* signed_ = new wxRadioButton(buttonsPanel, wxID_ANY, "Signed", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    wxRadioButton* unsigned_ = new wxRadioButton(buttonsPanel, wxID_ANY, "Unsigned");
    wxRadioButton* hexadecimal_ = new wxRadioButton(buttonsPanel, wxID_ANY, "Hexadecimal");
    dataSize->Add(signed_, 1, wxEXPAND, 1);
    dataSize->Add(unsigned_, 1, wxEXPAND, 1);
    dataSize->Add(hexadecimal_, 1, wxEXPAND, 1);

    dataSizer->Add(dataSize, 1, wxEXPAND, 5);


    wxStaticBoxSizer* dataDisplay = new wxStaticBoxSizer(wxVERTICAL, buttonsPanel, "Data size");
    wxRadioButton* byte1 = new wxRadioButton(buttonsPanel, wxID_ANY, "1 byte", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    wxRadioButton* byte2 = new wxRadioButton(buttonsPanel, wxID_ANY, "2 byte");
    wxRadioButton* byte4 = new wxRadioButton(buttonsPanel, wxID_ANY, "4 byte");
    wxCheckBox* checkMisaligned = new wxCheckBox(buttonsPanel, wxID_ANY, "Check misaligned");
    dataDisplay->Add(byte1, 1, 0, 2);
    dataDisplay->Add(byte2, 1, 0, 2);
    dataDisplay->Add(byte4, 1, 0, 2);
    dataDisplay->Add(checkMisaligned, 1, 0, 2);

    dataSizer->Add(dataDisplay, 1, wxEXPAND, 5);


    wxBoxSizer* right = new wxBoxSizer(wxVERTICAL);
    buttonsSizer->Add(right);
    wxStaticBoxSizer* compAgainst = new wxStaticBoxSizer(wxVERTICAL, buttonsPanel, "Compare to / by");
    right->Add(compAgainst, 2, wxEXPAND, 5);
    right->Add(dataSizer, 3, wxEXPAND, 5);

    wxBoxSizer* valueSizer = new wxBoxSizer(wxHORIZONTAL);
    wxTextCtrl* value = new wxTextCtrl(buttonsPanel, wxID_ANY);
    wxRadioButton* valueButton = new wxRadioButton(buttonsPanel, wxID_ANY, "Value", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    valueSizer->Add(valueButton, 1, wxEXPAND, 5);
    valueSizer->Add(value, 1, wxEXPAND, 5);
    compAgainst->Add(valueSizer, 1, wxEXPAND, 5);

    wxBoxSizer* addressSizer = new wxBoxSizer(wxHORIZONTAL);
    wxTextCtrl* address = new wxTextCtrl(buttonsPanel, wxID_ANY);
    wxRadioButton* addressButton = new wxRadioButton(buttonsPanel, wxID_ANY, "Address");
    addressSizer->Add(addressButton, 1, wxEXPAND, 5);
    addressSizer->Add(address, 1, wxEXPAND, 5);
    compAgainst->Add(addressSizer, 1, wxEXPAND, 5);


    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(ramSearchList, 1, wxEXPAND, 5);
    frameSizer->Add(buttonsPanel, 0, wxEXPAND, 5);
    SetSizer(frameSizer);

    renderTimer = new wxTimer(this);
    renderTimer->Start(16);
}

RAMSearchFrame::~RAMSearchFrame()
{
    mainFrame->ramSearchFrame = nullptr;
    delete ramSearchList;
    delete renderTimer;
}

void RAMSearchFrame::OnClose(wxCloseEvent& event)
{
    renderTimer->Stop();
    Destroy();
}

void RAMSearchFrame::RefreshLoop(wxTimerEvent& event)
{
    PaintEvent();
}

void RAMSearchFrame::PaintEvent()
{

}
