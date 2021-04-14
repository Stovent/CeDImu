#include "RAMSearchFrame.hpp"
#include "enums.hpp"
#include "../CeDImu.hpp"

#include <wx/panel.h>
#include <wx/sizer.h>

wxBEGIN_EVENT_TABLE(RAMSearchFrame, wxFrame)
    EVT_TIMER(wxID_ANY, RAMSearchFrame::RefreshLoop)
    EVT_CLOSE(RAMSearchFrame::OnClose)
    EVT_CHECKBOX(IDRAMSearchListCheckMisaligned, RAMSearchFrame::OnCheckMisaligned)
    EVT_RADIOBUTTON(IDRAMSearchListSigned, RAMSearchFrame::OnSigned)
    EVT_RADIOBUTTON(IDRAMSearchListUnsigned, RAMSearchFrame::OnUnsigned)
    EVT_RADIOBUTTON(IDRAMSearchListHexadecimal, RAMSearchFrame::OnHexadecimal)
    EVT_RADIOBUTTON(IDRAMSearchListByte1, RAMSearchFrame::OnByte1)
    EVT_RADIOBUTTON(IDRAMSearchListByte2, RAMSearchFrame::OnByte2)
    EVT_RADIOBUTTON(IDRAMSearchListByte4, RAMSearchFrame::OnByte4)
wxEND_EVENT_TABLE()

RAMSearchFrame::RAMSearchFrame(Board& baord, MainFrame* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "RAM Search", pos, size), board(baord)
{
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
    signed_ = new wxRadioButton(buttonsPanel, IDRAMSearchListSigned, "Signed", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    unsigned_ = new wxRadioButton(buttonsPanel, IDRAMSearchListUnsigned, "Unsigned");
    hexadecimal_ = new wxRadioButton(buttonsPanel, IDRAMSearchListHexadecimal, "Hexadecimal");
    dataSize->Add(signed_, 1, wxEXPAND, 1);
    dataSize->Add(unsigned_, 1, wxEXPAND, 1);
    dataSize->Add(hexadecimal_, 1, wxEXPAND, 1);

    dataSizer->Add(dataSize, 1, wxEXPAND, 5);


    wxStaticBoxSizer* dataDisplay = new wxStaticBoxSizer(wxVERTICAL, buttonsPanel, "Data size");
    byte1 = new wxRadioButton(buttonsPanel, IDRAMSearchListByte1, "1 byte", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    byte2 = new wxRadioButton(buttonsPanel, IDRAMSearchListByte2, "2 byte");
    byte4 = new wxRadioButton(buttonsPanel, IDRAMSearchListByte4, "4 byte");
    lastByte = byte1;
    checkMisaligned = new wxCheckBox(buttonsPanel, IDRAMSearchListCheckMisaligned, "Check misaligned");
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
    if(!mainFrame->pauseItem->IsChecked())
        ramSearchList->Refresh();
}

void RAMSearchFrame::OnCheckMisaligned(wxCommandEvent& event)
{
    if(checkMisaligned->GetValue())
    {
        ramSearchList->SetItemCount(mainFrame->app->cdi.board->GetRAMSize());
        if(lastByte == byte2)
        {
            ramSearchList->EnsureVisible(ramSearchList->GetTopItem() * 2 + ramSearchList->GetCountPerPage()-1);
        }
        else if (lastByte == byte4)
        {
            ramSearchList->EnsureVisible(ramSearchList->GetTopItem() * 4 + ramSearchList->GetCountPerPage()-1);
        }
    }
    else
    {
        if(lastByte == byte2)
        {
            ramSearchList->EnsureVisible(ramSearchList->GetTopItem() / 2);
            ramSearchList->SetItemCount(mainFrame->app->cdi.board->GetRAMSize() / 2);
        }
        else if (lastByte == byte4)
        {
            ramSearchList->EnsureVisible(ramSearchList->GetTopItem() / 4);
            ramSearchList->SetItemCount(mainFrame->app->cdi.board->GetRAMSize() / 4);
        }
        else
        {
            ramSearchList->SetItemCount(mainFrame->app->cdi.board->GetRAMSize());
        }
    }
    ramSearchList->Refresh();
}

void RAMSearchFrame::OnSigned(wxCommandEvent& event)
{
    ramSearchList->Refresh();
}

void RAMSearchFrame::OnUnsigned(wxCommandEvent& event)
{
    ramSearchList->Refresh();
}

void RAMSearchFrame::OnHexadecimal(wxCommandEvent& event)
{
    ramSearchList->Refresh();
}

void RAMSearchFrame::OnByte1(wxCommandEvent& event)
{
    if(!checkMisaligned->GetValue())
    {
        ramSearchList->SetItemCount(mainFrame->app->cdi.board->GetRAMSize());
        if(lastByte == byte2)
            ramSearchList->EnsureVisible(ramSearchList->GetTopItem() * 2 + ramSearchList->GetCountPerPage()-1);
        else
            ramSearchList->EnsureVisible(ramSearchList->GetTopItem() * 4 + ramSearchList->GetCountPerPage()-1);
    }

    lastByte = byte1;
    ramSearchList->Refresh();
}

void RAMSearchFrame::OnByte2(wxCommandEvent& event)
{
    if(!checkMisaligned->GetValue())
    {
        if(lastByte == byte1)
        {
            ramSearchList->EnsureVisible(ramSearchList->GetTopItem() / 2);
            ramSearchList->SetItemCount(mainFrame->app->cdi.board->GetRAMSize() / 2);
        }
        else
        {
            ramSearchList->SetItemCount(mainFrame->app->cdi.board->GetRAMSize() / 2);
            ramSearchList->EnsureVisible(ramSearchList->GetTopItem() * 2 + ramSearchList->GetCountPerPage()-1);
        }
    }

    lastByte = byte2;
    ramSearchList->Refresh();
}

void RAMSearchFrame::OnByte4(wxCommandEvent& event)
{
    if(!checkMisaligned->GetValue())
    {
        if(lastByte == byte1)
            ramSearchList->EnsureVisible(ramSearchList->GetTopItem() / 4);
        else
            ramSearchList->EnsureVisible(ramSearchList->GetTopItem() / 2);
        ramSearchList->SetItemCount(mainFrame->app->cdi.board->GetRAMSize() / 4);
    }

    lastByte = byte4;
    ramSearchList->Refresh();
}
