#include "VDSCViewer.hpp"
#include "MainFrame.hpp"
#include "../CeDImu.hpp"
#include "../CDI/common/utils.hpp"
#include "../CDI/common/Video.hpp"

#include <wx/button.h>
#include <wx/dirdlg.h>
#include <wx/dcclient.h>
#include <wx/image.h>
#include <wx/sizer.h>

wxBEGIN_EVENT_TABLE(VDSCViewer, wxFrame)
    EVT_TIMER(wxID_ANY, VDSCViewer::UpdateNotebook)
wxEND_EVENT_TABLE()

VDSCViewer::VDSCViewer(MainFrame* mainFrame, CeDImu& cedimu) :
    wxFrame(mainFrame, wxID_ANY, "VDSC Viewer", wxDefaultPosition, wxSize(800, 600)),
    m_cedimu(cedimu),
    m_mainFrame(mainFrame),
    m_updateTimer(this),
    m_flushIcadca(false),
    m_updateLists(false)
{
    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(frameSizer);
    m_notebook = new wxNotebook(this, wxID_ANY);
    frameSizer->Add(m_notebook, wxSizerFlags().Expand().Proportion(1));

    // Registers
    wxPanel* registersPage = new wxPanel(m_notebook);
    m_notebook->AddPage(registersPage, "Registers");
    wxBoxSizer* registersPageSizer = new wxBoxSizer(wxVERTICAL);
    registersPage->SetSizer(registersPageSizer);

    wxStaticBoxSizer* internalSizer = new wxStaticBoxSizer(wxHORIZONTAL, registersPage, "Internal registers");
    registersPageSizer->Add(internalSizer, wxSizerFlags().Expand());
    wxStaticBoxSizer* controlSizer = new wxStaticBoxSizer(wxHORIZONTAL, registersPage, "Control registers");
    registersPageSizer->Add(controlSizer, wxSizerFlags().Expand().Proportion(1));

    wxListItem nameCol;
    nameCol.SetId(0);
    nameCol.SetText("Name");
    nameCol.SetWidth(60);

    wxListItem addressCol;
    addressCol.SetId(1);
    addressCol.SetText("Address");
    addressCol.SetWidth(60);

    wxListItem valueCol;
    valueCol.SetId(2);
    valueCol.SetText("Value");
    valueCol.SetWidth(60);

    wxListItem disCol;
    disCol.SetId(3);
    disCol.SetText("Disassembled value");
    disCol.SetWidth(820);

    m_internalRegistersList = new wxListCtrl(registersPage, wxID_ANY, wxDefaultPosition, wxSize(-1, 234), wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);
    internalSizer->Add(m_internalRegistersList, wxSizerFlags().Expand());
    m_internalRegistersList->InsertColumn(0, nameCol);
    m_internalRegistersList->InsertColumn(1, addressCol);
    m_internalRegistersList->InsertColumn(2, valueCol);
    m_internalRegistersList->InsertColumn(3, disCol);

    nameCol.SetWidth(185);
    m_controlRegistersList = new wxListCtrl(registersPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);
    controlSizer->Add(m_controlRegistersList, wxSizerFlags().Expand());
    m_controlRegistersList->InsertColumn(0, nameCol);
    m_controlRegistersList->InsertColumn(1, addressCol);
    m_controlRegistersList->InsertColumn(2, valueCol);
    m_controlRegistersList->InsertColumn(3, disCol);

    // CA
    wxPanel* icadcaPage = new wxPanel(m_notebook);
    m_notebook->AddPage(icadcaPage, "ICA/DCA");
    wxBoxSizer* icadcaPageSizer = new wxBoxSizer(wxVERTICAL);
    icadcaPage->SetSizer(icadcaPageSizer);

    wxBoxSizer* dcaSizer = new wxBoxSizer(wxHORIZONTAL);
    icadcaPageSizer->Add(dcaSizer, wxSizerFlags().Expand().Proportion(1));
    wxBoxSizer* icaSizer = new wxBoxSizer(wxHORIZONTAL);
    icadcaPageSizer->Add(icaSizer, wxSizerFlags().Expand().Proportion(1));

    wxStaticBoxSizer* dca1Sizer = new wxStaticBoxSizer(wxVERTICAL, icadcaPage, "DCA 1");
    dcaSizer->Add(dca1Sizer, wxSizerFlags().Expand().Proportion(1));
    wxStaticBoxSizer* dca2Sizer = new wxStaticBoxSizer(wxVERTICAL, icadcaPage, "DCA 2");
    dcaSizer->Add(dca2Sizer, wxSizerFlags().Expand().Proportion(1));
    wxStaticBoxSizer* ica1Sizer = new wxStaticBoxSizer(wxVERTICAL, icadcaPage, "ICA 1");
    icaSizer->Add(ica1Sizer, wxSizerFlags().Expand().Proportion(1));
    wxStaticBoxSizer* ica2Sizer = new wxStaticBoxSizer(wxVERTICAL, icadcaPage, "ICA 2");
    icaSizer->Add(ica2Sizer, wxSizerFlags().Expand().Proportion(1));

    const std::function<void(wxListCtrl*)>icadcaListBuilder = [=] (wxListCtrl* list) {
        wxListItem col;
        col.SetId(0);
        col.SetText("Instruction");
        col.SetWidth(200);
        list->InsertColumn(0, col);
    };

    m_dca1List = new GenericList(icadcaPage, icadcaListBuilder, [=] (long item, long) -> wxString {
        std::lock_guard<std::mutex> lock(this->m_icadcaMutex);
        if(item >= (long)this->m_dca1.size())
            return "";
        return this->m_dca1[item];
    });
    dca1Sizer->Add(m_dca1List, wxSizerFlags().Expand().Proportion(1));

    m_ica1List = new GenericList(icadcaPage, icadcaListBuilder, [=] (long item, long) -> wxString {
        std::lock_guard<std::mutex> lock(this->m_icadcaMutex);
        if(item >= (long)this->m_ica1.size())
            return "";
        return this->m_ica1[item];
    });
    ica1Sizer->Add(m_ica1List, wxSizerFlags().Expand().Proportion(1));

    m_dca2List = new GenericList(icadcaPage, icadcaListBuilder, [=] (long item, long) -> wxString {
        std::lock_guard<std::mutex> lock(this->m_icadcaMutex);
        if(item >= (long)this->m_dca2.size())
            return "";
        return this->m_dca2[item];
    });
    dca2Sizer->Add(m_dca2List, wxSizerFlags().Expand().Proportion(1));

    m_ica2List = new GenericList(icadcaPage, icadcaListBuilder, [=] (long item, long) -> wxString {
        std::lock_guard<std::mutex> lock(this->m_icadcaMutex);
        if(item >= (long)this->m_ica2.size())
            return "";
        return this->m_ica2[item];
    });
    ica2Sizer->Add(m_ica2List, wxSizerFlags().Expand().Proportion(1));

    // Planes
    wxPanel* planesPage = new wxPanel(m_notebook);
    m_notebook->AddPage(planesPage, "Planes");
    wxBoxSizer* planesPageSizer = new wxBoxSizer(wxVERTICAL);
    planesPage->SetSizer(planesPageSizer);

    wxBoxSizer* planesPageTopSizer = new wxBoxSizer(wxHORIZONTAL);
    planesPageSizer->Add(planesPageTopSizer, wxSizerFlags().Expand().Proportion(1));
    wxBoxSizer* planesPageBotSizer = new wxBoxSizer(wxHORIZONTAL);
    planesPageSizer->Add(planesPageBotSizer, wxSizerFlags().Expand().Proportion(1));
    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    planesPageSizer->Add(buttonsSizer, wxSizerFlags().Expand());

    m_planeAPanel = new wxPanel(planesPage);
    wxStaticBoxSizer* planeASizer = new wxStaticBoxSizer(wxHORIZONTAL, planesPage, "Plane A");
    planesPageTopSizer->Add(planeASizer, wxSizerFlags().Expand().Proportion(1));
    planeASizer->Add(m_planeAPanel, wxSizerFlags().Expand().Proportion(1));

    m_planeBPanel = new wxPanel(planesPage);
    wxStaticBoxSizer* planeBSizer = new wxStaticBoxSizer(wxHORIZONTAL, planesPage, "Plane B");
    planesPageTopSizer->Add(planeBSizer, wxSizerFlags().Expand().Proportion(1));
    planeBSizer->Add(m_planeBPanel, wxSizerFlags().Expand().Proportion(1));

    m_cursorPanel = new wxPanel(planesPage);
    wxStaticBoxSizer* cursorSizer = new wxStaticBoxSizer(wxHORIZONTAL, planesPage, "Cursor");
    planesPageBotSizer->Add(cursorSizer, wxSizerFlags().Expand().Proportion(1));
    cursorSizer->Add(m_cursorPanel, wxSizerFlags().Expand().Proportion(1));

    m_backgdPanel = new wxPanel(planesPage);
    wxStaticBoxSizer* backgdSizer = new wxStaticBoxSizer(wxHORIZONTAL, planesPage, "Background");
    planesPageBotSizer->Add(backgdSizer, wxSizerFlags().Expand().Proportion(1));
    backgdSizer->Add(m_backgdPanel, wxSizerFlags().Expand().Proportion(1));

    wxButton* planeAButton = new wxButton(planesPage, wxID_ANY, "Save plane A");
    planeAButton->Bind(wxEVT_BUTTON, [=] (wxEvent&) {
        if(!this->m_imgPlaneA.IsOk())
            return;
        wxDirDialog dirDlg(this, wxDirSelectorPromptStr, wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        if(dirDlg.ShowModal() == wxID_OK)
        {
            std::lock_guard<std::mutex> lock(this->m_cedimu.m_cdiBoardMutex);
            if(this->m_cedimu.m_cdi.board)
            {
                uint32_t fc = this->m_cedimu.m_cdi.board->GetTotalFrameCount();
                std::lock_guard<std::mutex> lock2(m_imgMutex);
                this->m_imgPlaneA.SaveFile(dirDlg.GetPath().ToStdString() + "/planeA_" + std::to_string(fc) + ".png", wxBITMAP_TYPE_PNG);
            }
        }
    });
    buttonsSizer->Add(planeAButton, wxSizerFlags().Border().Proportion(1));

    wxButton* planeBButton = new wxButton(planesPage, wxID_ANY, "Save plane B");
    planeBButton->Bind(wxEVT_BUTTON, [=] (wxEvent&) {
        if(!this->m_imgPlaneB.IsOk())
            return;
        wxDirDialog dirDlg(this, wxDirSelectorPromptStr, wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        if(dirDlg.ShowModal() == wxID_OK)
        {
            std::lock_guard<std::mutex> lock(this->m_cedimu.m_cdiBoardMutex);
            if(this->m_cedimu.m_cdi.board)
            {
                uint32_t fc = this->m_cedimu.m_cdi.board->GetTotalFrameCount();
                std::lock_guard<std::mutex> lock2(m_imgMutex);
                this->m_imgPlaneB.SaveFile(dirDlg.GetPath().ToStdString() + "/planeB_" + std::to_string(fc) + ".png", wxBITMAP_TYPE_PNG);
            }
        }
    });
    buttonsSizer->Add(planeBButton, wxSizerFlags().Border().Proportion(1));

    wxButton* cursorButton = new wxButton(planesPage, wxID_ANY, "Save cursor");
    cursorButton->Bind(wxEVT_BUTTON, [=] (wxEvent&) {
        if(!this->m_imgCursor.IsOk())
            return;
        wxDirDialog dirDlg(this, wxDirSelectorPromptStr, wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        if(dirDlg.ShowModal() == wxID_OK)
        {
            std::lock_guard<std::mutex> lock(this->m_cedimu.m_cdiBoardMutex);
            if(this->m_cedimu.m_cdi.board)
            {
                uint32_t fc = this->m_cedimu.m_cdi.board->GetTotalFrameCount();
                std::lock_guard<std::mutex> lock2(m_imgMutex);
                this->m_imgCursor.SaveFile(dirDlg.GetPath().ToStdString() + "/cursor_" + std::to_string(fc) + ".png", wxBITMAP_TYPE_PNG);
            }
        }
    });
    buttonsSizer->Add(cursorButton, wxSizerFlags().Border().Proportion(1));

    wxButton* backgdButton = new wxButton(planesPage, wxID_ANY, "Save background");
    backgdButton->Bind(wxEVT_BUTTON, [=] (wxEvent&) {
        if(!this->m_imgBackgd.IsOk())
            return;
        wxDirDialog dirDlg(this, wxDirSelectorPromptStr, wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        if(dirDlg.ShowModal() == wxID_OK)
        {
            std::lock_guard<std::mutex> lock(this->m_cedimu.m_cdiBoardMutex);
            if(this->m_cedimu.m_cdi.board)
            {
                uint32_t fc = this->m_cedimu.m_cdi.board->GetTotalFrameCount();
                std::lock_guard<std::mutex> lock2(m_imgMutex);
                this->m_imgBackgd.SaveFile(dirDlg.GetPath().ToStdString() + "/background_" + std::to_string(fc) + ".png", wxBITMAP_TYPE_PNG);
            }
        }
    });
    buttonsSizer->Add(backgdButton, wxSizerFlags().Border().Proportion(1));

    Layout();
    Show();
    m_updateTimer.Start(16);

    m_cedimu.m_cdi.callbacks.SetOnLogICADCA([=] (ControlArea area, const std::string& inst) {
        std::lock_guard<std::mutex> lock(this->m_icadcaMutex);
        if(this->m_flushIcadca)
        {
            this->m_dca1.clear();
            this->m_ica1.clear();
            this->m_dca2.clear();
            this->m_ica2.clear();
            this->m_flushIcadca = false;
        }

        switch(area)
        {
            case ControlArea::DCA1:
                this->m_dca1.push_back(inst);
                break;
            case ControlArea::ICA1:
                this->m_ica1.push_back(inst);
                break;
            case ControlArea::DCA2:
                this->m_dca2.push_back(inst);
                break;
            case ControlArea::ICA2:
                this->m_ica2.push_back(inst);
                break;
        }
        this->m_updateLists = true;
    });
}

VDSCViewer::~VDSCViewer()
{
    m_cedimu.m_cdi.callbacks.SetOnLogICADCA(nullptr);
    m_mainFrame->m_vdscViewer = nullptr;
}

void VDSCViewer::UpdateNotebook(wxTimerEvent&)
{
    const int selectedPage = m_notebook->GetSelection();
    if(selectedPage == 0)
    {
        UpdateRegisters();
    }
    else if(selectedPage == 1)
    {
        if(!m_updateLists)
            return;
        m_updateLists = false;
        UpdateIcadca();
    }
    else if(selectedPage == 2)
    {
        UpdatePanels();
    }
}

void VDSCViewer::UpdateRegisters()
{
    std::unique_lock<std::mutex> lock(m_cedimu.m_cdiBoardMutex);
    if(!m_cedimu.m_cdi.board)
    {
        m_internalRegistersList->DeleteAllItems();
        m_controlRegistersList->DeleteAllItems();
        return;
    }

    std::vector<InternalRegister> iregs = m_cedimu.m_cdi.board->GetInternalRegisters();
    long i = 0;
    if(iregs.size() != (size_t)m_internalRegistersList->GetItemCount())
    {
        m_internalRegistersList->DeleteAllItems();
        for(const InternalRegister& reg : iregs)
        {
            m_internalRegistersList->InsertItem(i++, reg.name);
        }
    }
    else
    {
        for(const InternalRegister& reg : iregs)
        {
            m_internalRegistersList->SetItem(i, 1, toHex(reg.address));
            m_internalRegistersList->SetItem(i, 2, toHex(reg.value));
            m_internalRegistersList->SetItem(i, 3, reg.disassembledValue);
            i++;
        }
    }

    std::vector<InternalRegister> cregs = m_cedimu.m_cdi.board->GetControlRegisters();
    i = 0;
    if(cregs.size() != (size_t)m_controlRegistersList->GetItemCount())
    {
        m_controlRegistersList->DeleteAllItems();
        for(const InternalRegister& reg : cregs)
        {
            m_controlRegistersList->InsertItem(i++, reg.name);
        }
    }
    else
    {
        for(const InternalRegister& reg : cregs)
        {
            m_controlRegistersList->SetItem(i, 1, toHex(reg.address));
            m_controlRegistersList->SetItem(i, 2, toHex(reg.value));
            i++;
        }
    }
}

void VDSCViewer::UpdateIcadca()
{
    this->m_dca1List->SetItemCount(this->m_dca1.size());
    m_dca1List->Refresh();
    this->m_ica1List->SetItemCount(this->m_ica1.size());
    m_ica1List->Refresh();
    this->m_dca2List->SetItemCount(this->m_dca2.size());
    m_dca2List->Refresh();
    this->m_ica2List->SetItemCount(this->m_ica2.size());
    m_ica2List->Refresh();
}

void VDSCViewer::UpdatePanels()
{
    std::lock_guard<std::mutex> lock(m_cedimu.m_cdiBoardMutex);
    if(!m_cedimu.m_cdi.board)
        return;

    std::lock_guard<std::mutex> lock2(m_imgMutex);
    const Plane& planeA = m_cedimu.m_cdi.board->GetPlaneA();
    m_imgPlaneA.Create(planeA.width, planeA.height);
    if(m_imgPlaneA.IsOk())
    {
        if(!m_imgPlaneA.HasAlpha())
            m_imgPlaneA.InitAlpha();
        Video::splitARGB(planeA.data(), planeA.width * planeA.height * 4, m_imgPlaneA.GetAlpha(), m_imgPlaneA.GetData());
        const wxSize size = m_planeAPanel->GetClientSize();
        wxClientDC dc(m_planeAPanel);
        dc.Clear();
        dc.DrawBitmap(wxBitmap(m_imgPlaneA.Scale(size.x, size.y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
    }

    const Plane& planeB = m_cedimu.m_cdi.board->GetPlaneB();
    m_imgPlaneB.Create(planeB.width, planeB.height);
    if(m_imgPlaneB.IsOk())
    {
        if(!m_imgPlaneB.HasAlpha())
            m_imgPlaneB.InitAlpha();
        Video::splitARGB(planeB.data(), planeB.width * planeB.height * 4, m_imgPlaneB.GetAlpha(), m_imgPlaneB.GetData());
        const wxSize size = m_planeBPanel->GetClientSize();
        wxClientDC dc(m_planeBPanel);
        dc.Clear();
        dc.DrawBitmap(wxBitmap(m_imgPlaneB.Scale(size.x, size.y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
    }

    const Plane& cursor = m_cedimu.m_cdi.board->GetCursor();
    m_imgCursor.Create(cursor.width, cursor.height);
    if(m_imgCursor.IsOk())
    {
        if(!m_imgCursor.HasAlpha())
            m_imgCursor.InitAlpha();
        Video::splitARGB(cursor.data(), cursor.width * cursor.height * 4, m_imgCursor.GetAlpha(), m_imgCursor.GetData());
        wxClientDC dc(m_cursorPanel);
        dc.Clear();
        dc.DrawBitmap(wxBitmap(m_imgCursor), 0, 0);
    }

    const Plane& backgd = m_cedimu.m_cdi.board->GetBackground();
    m_imgBackgd.Create(backgd.width, backgd.height);
    if(m_imgBackgd.IsOk())
    {
        if(!m_imgBackgd.HasAlpha())
            m_imgBackgd.InitAlpha();
        Video::splitARGB(backgd.data(), backgd.width * backgd.height * 4, m_imgBackgd.GetAlpha(), m_imgBackgd.GetData());
        const wxSize size = m_backgdPanel->GetClientSize();
        wxClientDC dc(m_backgdPanel);
        dc.Clear();
        dc.DrawBitmap(wxBitmap(m_imgBackgd.Scale(size.x, size.y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
    }
}
