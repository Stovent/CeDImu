#include "VDSCViewer.hpp"
#include "MainFrame.hpp"
#include "../CeDImu.hpp"
#include "../CDI/common/utils.hpp"
#include "../CDI/common/Video.hpp"

#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/dcclient.h>
#include <wx/image.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>

wxBEGIN_EVENT_TABLE(VDSCViewer, wxFrame)
    EVT_CLOSE(VDSCViewer::OnClose)
    EVT_TIMER(wxID_ANY, VDSCViewer::UpdateNotebook)
wxEND_EVENT_TABLE()

VDSCViewer::VDSCViewer(MainFrame* mainFrame, CeDImu& cedimu)
    : wxFrame(mainFrame, wxID_ANY, "VDSC Viewer", wxDefaultPosition, wxSize(800, 600))
    , m_cedimu(cedimu)
    , m_mainFrame(mainFrame)
    , m_updateTimer(this)
    , m_flushIcadca(false)
    , m_updateLists(false)
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

    const std::function<void(wxListCtrl*)>dcaListBuilder = [] (wxListCtrl* list) {
        wxListItem frame;
        frame.SetId(0);
        frame.SetText("Frame");
        frame.SetWidth(50);
        list->InsertColumn(0, frame);

        wxListItem line;
        line.SetId(1);
        line.SetText("Line");
        line.SetWidth(35);
        list->InsertColumn(1, line);

        wxListItem inst;
        inst.SetId(3);
        inst.SetText("Instruction");
        inst.SetWidth(80);
        list->InsertColumn(3, inst);
    };

    const std::function<void(wxListCtrl*)>icaListBuilder = [] (wxListCtrl* list) {
        wxListItem frame;
        frame.SetId(0);
        frame.SetText("Frame");
        frame.SetWidth(50);
        list->InsertColumn(0, frame);

        wxListItem inst;
        inst.SetId(2);
        inst.SetText("Instruction");
        inst.SetWidth(80);
        list->InsertColumn(2, inst);
    };

    m_dca1List = new GenericList(icadcaPage, dcaListBuilder, [&] (long item, long col) -> wxString {
        std::lock_guard<std::mutex> lock(this->m_icadcaMutex);
        if(item >= as<long>(this->m_dca1.size()))
            return "";
        if(col == 0)
            return std::to_string(this->m_dca1[item].frame);
        if(col == 1)
            return std::to_string(this->m_dca1[item].line);
        return toHex(this->m_dca1[item].instruction);
    });
    dca1Sizer->Add(m_dca1List, wxSizerFlags().Expand().Proportion(1));

    m_ica1List = new GenericList(icadcaPage, icaListBuilder, [&] (long item, long col) -> wxString {
        std::lock_guard<std::mutex> lock(this->m_icadcaMutex);
        if(item >= as<long>(this->m_ica1.size()))
            return "";
        if(col == 0)
            return std::to_string(this->m_ica1[item].frame);
        return toHex(this->m_ica1[item].instruction);
    });
    ica1Sizer->Add(m_ica1List, wxSizerFlags().Expand().Proportion(1));

    m_dca2List = new GenericList(icadcaPage, dcaListBuilder, [&] (long item, long col) -> wxString {
        std::lock_guard<std::mutex> lock(this->m_icadcaMutex);
        if(item >= as<long>(this->m_dca2.size()))
            return "";
        if(col == 0)
            return std::to_string(this->m_dca2[item].frame);
        if(col == 1)
            return std::to_string(this->m_dca2[item].line);
        return toHex(this->m_dca2[item].instruction);
    });
    dca2Sizer->Add(m_dca2List, wxSizerFlags().Expand().Proportion(1));

    m_ica2List = new GenericList(icadcaPage, icaListBuilder, [&] (long item, long col) -> wxString {
        std::lock_guard<std::mutex> lock(this->m_icadcaMutex);
        if(item >= as<long>(this->m_ica2.size()))
            return "";
        if(col == 0)
            return std::to_string(this->m_ica2[item].frame);
        return toHex(this->m_ica2[item].instruction);
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
    planeAButton->Bind(wxEVT_BUTTON, [&] (wxEvent&) {
        if(!this->m_imgPlaneA.IsOk())
            return;

        std::lock_guard<std::recursive_mutex> lock(this->m_cedimu.m_cdiMutex);
        if(!this->m_cedimu.m_cdi)
            return;

        const bool isRunning = !this->m_mainFrame->m_pauseMenuItem->IsChecked();
        if(isRunning)
            this->m_cedimu.StopEmulation();
        uint32_t fc = this->m_cedimu.m_cdi->GetTotalFrameCount();

        wxFileDialog fileDlg(this, wxFileSelectorPromptStr, wxEmptyString, "planeA_" + std::to_string(fc) + ".png", "PNG (*.png)|*.png", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if(fileDlg.ShowModal() == wxID_OK)
        {
            std::lock_guard<std::mutex> lock2(m_imgMutex);
            if(!this->m_imgPlaneA.SaveFile(fileDlg.GetPath().ToStdString(), wxBITMAP_TYPE_PNG))
                wxMessageBox("Failed to save plane A");
        }

        if(isRunning)
            this->m_cedimu.StartEmulation();
    });
    buttonsSizer->Add(planeAButton, wxSizerFlags().Border().Proportion(1));

    wxButton* planeBButton = new wxButton(planesPage, wxID_ANY, "Save plane B");
    planeBButton->Bind(wxEVT_BUTTON, [&] (wxEvent&) {
        if(!this->m_imgPlaneB.IsOk())
            return;

        std::lock_guard<std::recursive_mutex> lock(this->m_cedimu.m_cdiMutex);
        if(!this->m_cedimu.m_cdi)
            return;

        const bool isRunning = !this->m_mainFrame->m_pauseMenuItem->IsChecked();
        if(isRunning)
            this->m_cedimu.StopEmulation();
        uint32_t fc = this->m_cedimu.m_cdi->GetTotalFrameCount();

        wxFileDialog fileDlg(this, wxFileSelectorPromptStr, wxEmptyString, "planeB_" + std::to_string(fc) + ".png", "PNG (*.png)|*.png", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if(fileDlg.ShowModal() == wxID_OK)
        {
            std::lock_guard<std::mutex> lock2(m_imgMutex);
            if(!this->m_imgPlaneB.SaveFile(fileDlg.GetPath().ToStdString(), wxBITMAP_TYPE_PNG))
                wxMessageBox("Failed to save plane B");
        }

        if(isRunning)
            this->m_cedimu.StartEmulation();
    });
    buttonsSizer->Add(planeBButton, wxSizerFlags().Border().Proportion(1));

    wxButton* cursorButton = new wxButton(planesPage, wxID_ANY, "Save cursor");
    cursorButton->Bind(wxEVT_BUTTON, [&] (wxEvent&) {
        if(!this->m_imgCursor.IsOk())
            return;

        std::lock_guard<std::recursive_mutex> lock(this->m_cedimu.m_cdiMutex);
        if(!this->m_cedimu.m_cdi)
            return;

        const bool isRunning = !this->m_mainFrame->m_pauseMenuItem->IsChecked();
        if(isRunning)
            this->m_cedimu.StopEmulation();
        uint32_t fc = this->m_cedimu.m_cdi->GetTotalFrameCount();

        wxFileDialog fileDlg(this, wxFileSelectorPromptStr, wxEmptyString, "cursor_" + std::to_string(fc) + ".png", "PNG (*.png)|*.png", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if(fileDlg.ShowModal() == wxID_OK)
        {
            std::lock_guard<std::mutex> lock2(m_imgMutex);
            if(!this->m_imgCursor.SaveFile(fileDlg.GetPath().ToStdString(), wxBITMAP_TYPE_PNG))
                wxMessageBox("Failed to save cursor");
        }

        if(isRunning)
            this->m_cedimu.StartEmulation();
    });
    buttonsSizer->Add(cursorButton, wxSizerFlags().Border().Proportion(1));

    wxButton* backgdButton = new wxButton(planesPage, wxID_ANY, "Save background");
    backgdButton->Bind(wxEVT_BUTTON, [&] (wxEvent&) {
        if(!this->m_imgBackgd.IsOk())
            return;

        std::lock_guard<std::recursive_mutex> lock(this->m_cedimu.m_cdiMutex);
        if(!this->m_cedimu.m_cdi)
            return;

        const bool isRunning = !this->m_mainFrame->m_pauseMenuItem->IsChecked();
        if(isRunning)
            this->m_cedimu.StopEmulation();
        uint32_t fc = this->m_cedimu.m_cdi->GetTotalFrameCount();

        wxFileDialog fileDlg(this, wxFileSelectorPromptStr, wxEmptyString, "background_" + std::to_string(fc) + ".png", "PNG (*.png)|*.png", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if(fileDlg.ShowModal() == wxID_OK)
        {
            std::lock_guard<std::mutex> lock2(m_imgMutex);
            if(!this->m_imgBackgd.SaveFile(fileDlg.GetPath().ToStdString(), wxBITMAP_TYPE_PNG))
                wxMessageBox("Failed to save background");
        }

        if(isRunning)
            this->m_cedimu.StartEmulation();
    });
    buttonsSizer->Add(backgdButton, wxSizerFlags().Border().Proportion(1));

    Layout();
    Show();
    m_updateTimer.Start(16);

    m_cedimu.SetOnLogICADCA([&] (Video::ControlArea area, LogICADCA inst) {
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
            case Video::ControlArea::DCA1:
                this->m_dca1.push_back(inst);
                break;
            case Video::ControlArea::ICA1:
                this->m_ica1.push_back(inst);
                break;
            case Video::ControlArea::DCA2:
                this->m_dca2.push_back(inst);
                break;
            case Video::ControlArea::ICA2:
                this->m_ica2.push_back(inst);
                break;
        }
        this->m_updateLists = true;
    });
}

VDSCViewer::~VDSCViewer()
{
    m_mainFrame->m_vdscViewer = nullptr;
}

void VDSCViewer::OnClose(wxCloseEvent&)
{
    m_updateTimer.Stop();
    m_cedimu.SetOnLogICADCA(nullptr);
    Destroy();
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
    std::unique_lock<std::recursive_mutex> lock(m_cedimu.m_cdiMutex);
    if(!m_cedimu.m_cdi)
    {
        m_internalRegistersList->DeleteAllItems();
        m_controlRegistersList->DeleteAllItems();
        return;
    }

    std::vector<InternalRegister> iregs = m_cedimu.m_cdi->GetVDSCInternalRegisters();
    long i = 0;
    if(iregs.size() != as<size_t>(m_internalRegistersList->GetItemCount()))
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

    std::vector<InternalRegister> cregs = m_cedimu.m_cdi->GetVDSCControlRegisters();
    i = 0;
    if(cregs.size() != as<size_t>(m_controlRegistersList->GetItemCount()))
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
    std::lock_guard<std::recursive_mutex> lock(m_cedimu.m_cdiMutex);
    if(!m_cedimu.m_cdi)
        return;

    std::lock_guard<std::mutex> lock2(m_imgMutex);
    const Video::Plane& planeA = m_cedimu.m_cdi->GetPlaneA();
    m_imgPlaneA.Create(planeA.m_width, planeA.m_height);
    if(m_imgPlaneA.IsOk())
    {
        if(!m_imgPlaneA.HasAlpha())
            m_imgPlaneA.InitAlpha();
        Video::splitARGB(planeA.data(), planeA.m_width * planeA.m_height * 4, m_imgPlaneA.GetAlpha(), m_imgPlaneA.GetData());
        const wxSize size = m_planeAPanel->GetClientSize();
        wxClientDC dc(m_planeAPanel);
        dc.Clear();
        dc.DrawBitmap(wxBitmap(m_imgPlaneA.Scale(size.x, size.y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
    }

    const Video::Plane& planeB = m_cedimu.m_cdi->GetPlaneB();
    m_imgPlaneB.Create(planeB.m_width, planeB.m_height);
    if(m_imgPlaneB.IsOk())
    {
        if(!m_imgPlaneB.HasAlpha())
            m_imgPlaneB.InitAlpha();
        Video::splitARGB(planeB.data(), planeB.m_width * planeB.m_height * 4, m_imgPlaneB.GetAlpha(), m_imgPlaneB.GetData());
        const wxSize size = m_planeBPanel->GetClientSize();
        wxClientDC dc(m_planeBPanel);
        dc.Clear();
        dc.DrawBitmap(wxBitmap(m_imgPlaneB.Scale(size.x, size.y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
    }

    const Video::Plane& cursor = m_cedimu.m_cdi->GetCursor();
    m_imgCursor.Create(cursor.m_width, cursor.m_height);
    if(m_imgCursor.IsOk())
    {
        if(!m_imgCursor.HasAlpha())
            m_imgCursor.InitAlpha();
        Video::splitARGB(cursor.data(), cursor.m_width * cursor.m_height * 4, m_imgCursor.GetAlpha(), m_imgCursor.GetData());
        wxClientDC dc(m_cursorPanel);
        dc.Clear();
        dc.DrawBitmap(wxBitmap(m_imgCursor), 0, 0);
    }

    const Video::Plane& backgd = m_cedimu.m_cdi->GetBackground();
    m_imgBackgd.Create(backgd.m_width, backgd.m_height);
    if(m_imgBackgd.IsOk())
    {
        memcpy(m_imgBackgd.GetData(), backgd.data(), backgd.m_width * backgd.m_height * 3);

        const wxSize size = m_backgdPanel->GetClientSize();
        wxClientDC dc(m_backgdPanel);
        dc.Clear();
        dc.DrawBitmap(wxBitmap(m_imgBackgd.Scale(size.x, size.y, wxIMAGE_QUALITY_NEAREST)), 0, 0);
    }
}
