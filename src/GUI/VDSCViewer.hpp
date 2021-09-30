#ifndef GUI_VDSCVIEWER_HPP
#define GUI_VDSCVIEWER_HPP

class CeDImu;
class MainFrame;
#include "GenericList.hpp"

#include <wx/image.h>
#include <wx/frame.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/timer.h>

#include <mutex>
#include <vector>

class VDSCViewer : public wxFrame
{
public:
    CeDImu& m_cedimu;
    MainFrame* m_mainFrame;

    wxTimer m_updateTimer;
    wxNotebook* m_notebook;

    wxListCtrl* m_internalRegistersList;
    wxListCtrl* m_controlRegistersList;

    bool m_flushIcadca;
    bool m_updateLists;
    std::mutex m_icadcaMutex;
    std::vector<std::string> m_dca1;
    std::vector<std::string> m_ica1;
    std::vector<std::string> m_dca2;
    std::vector<std::string> m_ica2;
    GenericList* m_dca1List;
    GenericList* m_ica1List;
    GenericList* m_dca2List;
    GenericList* m_ica2List;

    wxPanel* m_planeAPanel;
    wxPanel* m_planeBPanel;
    wxPanel* m_cursorPanel;
    wxPanel* m_backgdPanel;
    std::mutex m_imgMutex;
    wxImage m_imgPlaneA;
    wxImage m_imgPlaneB;
    wxImage m_imgCursor;
    wxImage m_imgBackgd;

    VDSCViewer() = delete;
    VDSCViewer(MainFrame* mainFrame, CeDImu& cedimu);
    ~VDSCViewer();

    void UpdateNotebook(wxTimerEvent&);
    void UpdateRegisters();
    void UpdateIcadca();
    void UpdatePanels();

    wxDECLARE_EVENT_TABLE();
};

#endif // GUI_VDSCVIEWER_HPP
