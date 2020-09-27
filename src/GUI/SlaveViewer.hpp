#ifndef SLAVEVIEWER_HPP
#define SLAVEVIEWER_HPP

class SlaveViewer;

#include "../Boards/Board.hpp"
class MainFrame;

#include <wx/frame.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/notebook.h>
#include <wx/textctrl.h>
#include <wx/timer.h>


class SlaveViewer : public wxFrame
{
    MainFrame* mainFrame;
    Board* board;
    wxTimer timer;
    wxNotebook* notebook;
    wxListCtrl* internalList;
    wxListCtrl* controlList;

public:
    SlaveViewer(MainFrame* parent, Board* board);
    ~SlaveViewer();

    void RefreshLoop(wxTimerEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // SLAVEVIEWER_HPP
