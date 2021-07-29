#ifndef VDSCVIEWER_HPP
#define VDSCVIEWER_HPP

class CDI;
class MainFrame;
#include "GenericList.hpp"

#include <wx/frame.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/timer.h>

#include <mutex>
#include <vector>

class VDSCViewer : public wxFrame
{
    std::vector<std::string> ICA1;
    std::vector<std::string> DCA1;
    std::vector<std::string> ICA2;
    std::vector<std::string> DCA2;

    MainFrame* mainFrame;
    CDI& cdi;
    wxTimer timer;
    wxNotebook* notebook;
    wxListCtrl* internalList;
    wxListCtrl* controlList;
    wxPanel* planeAPanel;
    wxPanel* planeBPanel;
    wxPanel* cursorPanel;
    wxPanel* backgroundPanel;
    GenericList* ica1List;
    GenericList* dca1List;
    GenericList* ica2List;
    GenericList* dca2List;

public:
    std::mutex caMutex;
    bool flushICADCA;

    VDSCViewer(MainFrame* parent, CDI& idc);
    ~VDSCViewer();

    void RefreshLoop(wxTimerEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // VDSCVIEWER_HPP
