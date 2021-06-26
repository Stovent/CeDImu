#include "VDSCViewer.hpp"
#include "enums.hpp"
#include "MainFrame.hpp"
#include "../CDI/CDI.hpp"
#include "../CDI/common/utils.hpp"
#include "../CDI/common/Video.hpp"
#include "../CDI/cores/VDSC.hpp"

#include <wx/dcclient.h>

#include <algorithm>
#include <iterator>
#include <sstream>

wxBEGIN_EVENT_TABLE(VDSCViewer, wxFrame)
    EVT_TIMER(IDVDSCViewerTimer, VDSCViewer::RefreshLoop)
wxEND_EVENT_TABLE()

VDSCViewer::VDSCViewer(MainFrame* parent, CDI& idc) : wxFrame(parent, wxID_ANY, "VDSC Viewer"), cdi(idc), timer(this, IDVDSCViewerTimer)
{
    mainFrame = parent;

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

        std::vector<VDSCRegister> iregs = cdi.board->GetInternalRegisters();
        long i = 0;
        for(const VDSCRegister& reg : iregs)
        {
            long itemIndex = internalList->InsertItem(i++, reg.name);
            internalList->SetItem(itemIndex, 1, toHex(reg.address));
            internalList->SetItem(itemIndex, 2, toHex(reg.value));
            internalList->SetItem(itemIndex, 3, reg.disassembledValue);
        }

        std::vector<VDSCRegister> cregs = cdi.board->GetControlRegisters();
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
        const std::function<void(wxListCtrl*)> builder = [=] (wxListCtrl* self) {
            wxListItem col;
            col.SetId(0);
            col.SetText("Instruction");
            col.SetWidth(400);
            self->InsertColumn(0, col);
        };
        ica1List = new GenericList(icadcaPage, builder, [=] (unsigned long item, long) -> std::string { std::lock_guard<std::mutex> lock(this->caMutex); if(item >= this->ICA1.size()) return ""; return this->ICA1[item]; });
        dca1List = new GenericList(icadcaPage, builder, [=] (unsigned long item, long) -> std::string { std::lock_guard<std::mutex> lock(this->caMutex); if(item >= this->DCA1.size()) return ""; return this->DCA1[item]; });
        ica2List = new GenericList(icadcaPage, builder, [=] (unsigned long item, long) -> std::string { std::lock_guard<std::mutex> lock(this->caMutex); if(item >= this->ICA2.size()) return ""; return this->ICA2[item]; });
        dca2List = new GenericList(icadcaPage, builder, [=] (unsigned long item, long) -> std::string { std::lock_guard<std::mutex> lock(this->caMutex); if(item >= this->DCA2.size()) return ""; return this->DCA2[item]; });

        wxStaticBoxSizer* ica1Sizer = new wxStaticBoxSizer(wxHORIZONTAL, icadcaPage, "ICA 1");
        wxStaticBoxSizer* dca1Sizer = new wxStaticBoxSizer(wxHORIZONTAL, icadcaPage, "DCA 1");
        wxStaticBoxSizer* ica2Sizer = new wxStaticBoxSizer(wxHORIZONTAL, icadcaPage, "ICA 2");
        wxStaticBoxSizer* dca2Sizer = new wxStaticBoxSizer(wxHORIZONTAL, icadcaPage, "DCA 2");

        ica1Sizer->Add(ica1List, 1, wxEXPAND);
        dca1Sizer->Add(dca1List, 1, wxEXPAND);
        ica2Sizer->Add(ica2List, 1, wxEXPAND);
        dca2Sizer->Add(dca2List, 1, wxEXPAND);

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

    cdi.callbacks.SetOnLogICADCA([=] (ControlArea area, const std::string& str) {
        std::lock_guard<std::mutex> lock(this->caMutex);
        switch(area)
        {
        case ica1:
            if(this->flushICA1)
            {
                this->flushICA1 = false;
                this->ICA1.clear();
            }
            this->ICA1.push_back(str);
            break;

        case dca1:
            if(this->flushDCA1)
            {
                this->flushDCA1 = false;
                this->DCA1.clear();
            }
            this->DCA1.push_back(str);
            break;

        case ica2:
            if(this->flushICA2)
            {
                this->flushICA2 = false;
                this->ICA2.clear();
            }
            this->ICA2.push_back(str);
            break;

        case dca2:
            if(this->flushDCA2)
            {
                this->flushDCA2 = false;
                this->DCA2.clear();
            }
            this->DCA2.push_back(str);
            break;
        }
    });

    timer.Start(16);
}

VDSCViewer::~VDSCViewer()
{
    cdi.callbacks.SetOnLogICADCA(nullptr);
    mainFrame->vdscViewer = nullptr;
}

void VDSCViewer::RefreshLoop(wxTimerEvent& event)
{
    const int selectedPage = notebook->GetSelection();
    if(selectedPage == 0) // Registers
    {
        std::vector<VDSCRegister> iregs = cdi.board->GetInternalRegisters();
        long i = 0;
        for(const VDSCRegister& reg : iregs)
        {
            internalList->SetItem(i, 2, toHex(reg.value));
            internalList->SetItem(i++, 3, reg.disassembledValue);
        }

        std::vector<VDSCRegister> cregs = cdi.board->GetControlRegisters();
        i = 0;
        for(const VDSCRegister& reg : cregs)
        {
            controlList->SetItem(i++, 2, toHex(reg.value));
        }
    }
    else if(selectedPage == 1) // ICA/DCA
    {
        std::lock_guard<std::mutex> lock(caMutex);
        ica1List->SetItemCount(ICA1.size());
        ica1List->Refresh();
        dca1List->SetItemCount(DCA1.size());
        dca1List->Refresh();
        ica2List->SetItemCount(ICA2.size());
        ica2List->Refresh();
        dca2List->SetItemCount(DCA2.size());
        dca2List->Refresh();
    }
    else if(selectedPage == 2) // Planes
    {
        wxClientDC dcA(planeAPanel);
        wxClientDC dcB(planeBPanel);
        wxClientDC dcBackground(backgroundPanel);
        wxClientDC dcCursor(cursorPanel);

        const Plane& a = cdi.board->GetPlaneA();
        const Plane& b = cdi.board->GetPlaneB();
        const Plane& bg = cdi.board->GetBackground();
        const Plane& c = cdi.board->GetCursor();

        if(a.width && a.height)
        {
            wxImage planeA(a.width, a.height);
            if(!planeA.HasAlpha())
                planeA.InitAlpha();
            Video::splitARGB(a.data(), a.width * a.height * 4, planeA.GetAlpha(), planeA.GetData());
            if(planeA.IsOk())
                dcA.DrawBitmap(wxBitmap(planeA.Scale(planeAPanel->GetClientSize().x, planeAPanel->GetClientSize().y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
        }

        if(b.width && b.height)
        {
            wxImage planeB(b.width, b.height);
            if(!planeB.HasAlpha())
                planeB.InitAlpha();
            Video::splitARGB(b.data(), b.width * b.height * 4, planeB.GetAlpha(), planeB.GetData());
            if(planeB.IsOk())
                dcB.DrawBitmap(wxBitmap(planeB.Scale(planeBPanel->GetClientSize().x, planeBPanel->GetClientSize().y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
        }

        if(bg.width && bg.height)
        {
            wxImage background(bg.width, bg.height);
            if(!background.HasAlpha())
                background.InitAlpha();
            Video::splitARGB(bg.data(), bg.width * bg.height * 4, background.GetAlpha(), background.GetData());
            if(background.IsOk())
                dcBackground.DrawBitmap(wxBitmap(background.Scale(backgroundPanel->GetClientSize().x, backgroundPanel->GetClientSize().y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
        }

        if(c.width && c.height)
        {
            wxImage cursor(c.width, c.height);
            if(!cursor.HasAlpha())
                cursor.InitAlpha();
            Video::splitARGB(c.data(), c.width *c.height * 4, cursor.GetAlpha(), cursor.GetData());
            if(cursor.IsOk())
                dcCursor.DrawBitmap(wxBitmap(cursor), 0, 0);
        }
    }
}
