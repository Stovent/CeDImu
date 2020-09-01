#include "VDSCViewer.hpp"

#include <algorithm>
#include <iterator>
#include <sstream>

#include <wx/dcclient.h>

#include "../utils.hpp"

wxBEGIN_EVENT_TABLE(VDSCViewer, wxFrame)
    EVT_TIMER(IDVDSCViewerTimer, VDSCViewer::RefreshLoop)
wxEND_EVENT_TABLE()

VDSCViewer::VDSCViewer(MainFrame* parent, Board* board) : wxFrame(parent, wxID_ANY, "VDSC Viewer"), timer(this, IDVDSCViewerTimer)
{
    mainFrame = parent;
    this->board = board;
    timer.Start(16);

    wxPanel* notebookPanel = new wxPanel(this);
    notebook = new wxNotebook(notebookPanel, wxID_ANY);

    // Registers
    wxNotebookPage* registersPage = new wxNotebookPage(notebook, wxID_ANY);
    {
        wxBoxSizer* listsSizer = new wxBoxSizer(wxVERTICAL);
        internalList = new wxListCtrl(registersPage, wxID_ANY, wxDefaultPosition, wxSize(300, 234), wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);
        wxListItem nameCol;
        nameCol.SetId(0);
        nameCol.SetText("Name");
        nameCol.SetWidth(60);
        internalList->InsertColumn(0, nameCol);

        wxListItem addressCol;
        addressCol.SetId(1);
        addressCol.SetText("Address");
        addressCol.SetWidth(60);
        internalList->InsertColumn(1, addressCol);

        wxListItem valueCol;
        valueCol.SetId(2);
        valueCol.SetText("Value");
        valueCol.SetWidth(60);
        internalList->InsertColumn(2, valueCol);

        wxListItem disCol;
        disCol.SetId(3);
        disCol.SetText("Disassembled value");
        disCol.SetWidth(820);
        internalList->InsertColumn(3, disCol);

        listsSizer->Add(internalList, 0, wxEXPAND);

        controlList = new wxListCtrl(registersPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);

        wxListItem nameCol2;
        nameCol2.SetId(0);
        nameCol2.SetText("Name");
        nameCol2.SetWidth(200);
        controlList->InsertColumn(0, nameCol2);

        wxListItem addressCol2;
        addressCol2.SetId(1);
        addressCol2.SetText("Address");
        addressCol2.SetWidth(60);
        controlList->InsertColumn(1, addressCol2);

        wxListItem valueCol2;
        valueCol2.SetId(2);
        valueCol2.SetText("Value");
        valueCol2.SetWidth(100);
        controlList->InsertColumn(2, valueCol2);

        listsSizer->Add(controlList, 1, wxEXPAND);

        registersPage->SetSizer(listsSizer);

        std::vector<VDSCRegister> iregs = board->GetInternalRegisters();
        long i = 0;
        for(const VDSCRegister& reg : iregs)
        {
            long itemIndex = internalList->InsertItem(i++, reg.name);
            internalList->SetItem(itemIndex, 1, toHex(reg.address));
            internalList->SetItem(itemIndex, 2, toHex(reg.value));
            internalList->SetItem(itemIndex, 3, reg.disassembledValue);
        }

        std::vector<VDSCRegister> cregs = board->GetControlRegisters();
        i = 0;
        for(const VDSCRegister& reg : cregs)
        {
            long itemIndex = controlList->InsertItem(i++, reg.name);
            controlList->SetItem(itemIndex, 1, toHex(reg.address));
            controlList->SetItem(itemIndex, 2, toHex(reg.value));
        }
    }
    notebook->AddPage(registersPage, "Registers");


    // ICA/DCA
    wxNotebookPage* icadcaPage = new wxNotebookPage(notebook, wxID_ANY);
    {
        ica1Text = new wxTextCtrl(icadcaPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
        dca1Text = new wxTextCtrl(icadcaPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
        ica2Text = new wxTextCtrl(icadcaPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
        dca2Text = new wxTextCtrl(icadcaPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);

        wxStaticBoxSizer* ica1Sizer = new wxStaticBoxSizer(wxHORIZONTAL, icadcaPage, "ICA 1");
        wxStaticBoxSizer* dca1Sizer = new wxStaticBoxSizer(wxHORIZONTAL, icadcaPage, "DCA 1");
        wxStaticBoxSizer* ica2Sizer = new wxStaticBoxSizer(wxHORIZONTAL, icadcaPage, "ICA 2");
        wxStaticBoxSizer* dca2Sizer = new wxStaticBoxSizer(wxHORIZONTAL, icadcaPage, "DCA 2");

        ica1Sizer->Add(ica1Text, 1, wxEXPAND);
        dca1Sizer->Add(dca1Text, 1, wxEXPAND);
        ica2Sizer->Add(ica2Text, 1, wxEXPAND);
        dca2Sizer->Add(dca2Text, 1, wxEXPAND);

        wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
        topSizer->Add(ica1Sizer, 1, wxEXPAND);
        topSizer->Add(dca1Sizer, 1, wxEXPAND);

        wxBoxSizer* botSizer = new wxBoxSizer(wxHORIZONTAL);
        botSizer->Add(ica2Sizer, 1, wxEXPAND);
        botSizer->Add(dca2Sizer, 1, wxEXPAND);

        wxBoxSizer* topBotSizer = new wxBoxSizer(wxVERTICAL);
        topBotSizer->Add(topSizer, 1, wxEXPAND);
        topBotSizer->Add(botSizer, 1, wxEXPAND);

        icadcaPage->SetSizer(topBotSizer);
    }
    notebook->AddPage(icadcaPage, "ICA/DCA");

    // Planes
    wxNotebookPage* planesPage = new wxNotebookPage(notebook, wxID_ANY);
    {
        planeAPanel = new wxPanel(planesPage);
        planeBPanel = new wxPanel(planesPage);
        cursorPanel = new wxPanel(planesPage);
        backgroundPanel = new wxPanel(planesPage);

        wxStaticBoxSizer* planeASizer = new wxStaticBoxSizer(wxHORIZONTAL, planesPage, "Plane A");
        wxStaticBoxSizer* planeBSizer = new wxStaticBoxSizer(wxHORIZONTAL, planesPage, "Plane B");
        wxStaticBoxSizer* cursorSizer = new wxStaticBoxSizer(wxHORIZONTAL, planesPage, "Cursor");
        wxStaticBoxSizer* backgroundSizer = new wxStaticBoxSizer(wxHORIZONTAL, planesPage, "Background");

        planeASizer->Add(planeAPanel, 1, wxEXPAND);
        planeBSizer->Add(planeBPanel, 1, wxEXPAND);
        cursorSizer->Add(cursorPanel, 1, wxEXPAND);
        backgroundSizer->Add(backgroundPanel, 1, wxEXPAND);

        wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
        topSizer->Add(planeASizer, 1, wxEXPAND);
        topSizer->Add(planeBSizer, 1, wxEXPAND);

        wxBoxSizer* botSizer = new wxBoxSizer(wxHORIZONTAL);
        botSizer->Add(cursorSizer, 1, wxEXPAND);
        botSizer->Add(backgroundSizer, 1, wxEXPAND);

        wxBoxSizer* topBotSizer = new wxBoxSizer(wxVERTICAL);
        topBotSizer->Add(topSizer, 1, wxEXPAND);
        topBotSizer->Add(botSizer, 1, wxEXPAND);

        planesPage->SetSizer(topBotSizer);
    }
    notebook->AddPage(planesPage, "Planes");


    wxBoxSizer* notebookSizer = new wxBoxSizer(wxVERTICAL);
    notebookSizer->Add(notebook, 1, wxEXPAND);

    notebookPanel->SetSizer(notebookSizer);
}

VDSCViewer::~VDSCViewer()
{
    mainFrame->vdscViewer = nullptr;
}

void VDSCViewer::RefreshLoop(wxTimerEvent& event)
{
    const int selectedPage = notebook->GetSelection();
    if(selectedPage == 0) // Registers
    {
        std::vector<VDSCRegister> iregs = board->GetInternalRegisters();
        long i = 0;
        for(const VDSCRegister& reg : iregs)
        {
            internalList->SetItem(i, 2, toHex(reg.value));
            internalList->SetItem(i++, 3, reg.disassembledValue);
        }

        std::vector<VDSCRegister> cregs = board->GetControlRegisters();
        i = 0;
        for(const VDSCRegister& reg : cregs)
        {
            controlList->SetItem(i++, 2, toHex(reg.value));
        }
    }
    else if(selectedPage == 1) // ICA/DCA
    {
        std::stringstream ica1;
        std::ostream_iterator<std::string> ssica1(ica1, "\n");
        std::vector<std::string> vica1 = board->GetICA1();
        std::copy(vica1.begin(), vica1.end(), ssica1);
        ica1Text->SetValue(ica1.str());

        std::stringstream dca1;
        std::ostream_iterator<std::string> ssdca1(dca1, "\n");
        std::vector<std::string> vdca1 = board->GetDCA1();
        std::copy(vdca1.begin(), vdca1.end(), ssdca1);
        dca1Text->SetValue(dca1.str());

        std::stringstream ica2;
        std::ostream_iterator<std::string> ssica2(ica2, "\n");
        std::vector<std::string> vica2 = board->GetICA2();
        std::copy(vica2.begin(), vica2.end(), ssica2);
        ica2Text->SetValue(ica2.str());

        std::stringstream dca2;
        std::ostream_iterator<std::string> ssdca2(dca2, "\n");
        std::vector<std::string> vdca2 = board->GetDCA2();
        std::copy(vdca2.begin(), vdca2.end(), ssdca2);
        dca2Text->SetValue(dca2.str());
    }
    else if(selectedPage == 2) // Planes
    {
        wxClientDC dcA(planeAPanel);
        wxClientDC dcB(planeBPanel);
        wxClientDC dcBackground(backgroundPanel);
        wxClientDC dcCursor(cursorPanel);

        wxImage planeA = board->GetPlaneA();
        wxImage planeB = board->GetPlaneB();
        wxImage background = board->GetBackground();
        wxImage cursor = board->GetCursor();

        if(planeA.IsOk())
            dcA.DrawBitmap(wxBitmap(planeA.Scale(planeAPanel->GetClientSize().x, planeAPanel->GetClientSize().y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
        if(planeB.IsOk())
            dcB.DrawBitmap(wxBitmap(planeB.Scale(planeBPanel->GetClientSize().x, planeBPanel->GetClientSize().y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
        if(background.IsOk())
            dcBackground.DrawBitmap(wxBitmap(background.Scale(backgroundPanel->GetClientSize().x, backgroundPanel->GetClientSize().y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
        if(cursor.IsOk())
            dcCursor.DrawBitmap(wxBitmap(cursor), 0, 0);
    }
}
