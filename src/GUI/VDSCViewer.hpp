#ifndef VDSCVIEWER_HPP
#define VDSCVIEWER_HPP

class VDSCViewer;

#include <wx/frame.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/notebook.h>
#include <wx/textctrl.h>
#include <wx/timer.h>

#include "../cores/VDSC.hpp"

class VDSCViewer : public wxFrame
{
    MainFrame* mainFrame;
    VDSC* vdsc;
    wxTimer timer;
    wxNotebook* notebook;
    wxListCtrl* internalList;
    wxListCtrl* controlList;
    wxPanel* planeAPanel;
    wxPanel* planeBPanel;
    wxPanel* cursorPanel;
    wxPanel* backgroundPanel;
    wxTextCtrl* ica1Text;
    wxTextCtrl* dca1Text;
    wxTextCtrl* ica2Text;
    wxTextCtrl* dca2Text;

public:
    VDSCViewer(MainFrame* parent, VDSC* gpu);
    ~VDSCViewer();

    void RefreshLoop(wxTimerEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // VDSCVIEWER_HPP
