#ifndef SLAVEVIEWER_HPP
#define SLAVEVIEWER_HPP

class SlaveViewer;

class MainFrame;
#include "GenericList.hpp"
#include "../CDI/boards/Board.hpp"

#include <wx/frame.h>
#include <wx/notebook.h>
#include <wx/timer.h>


class SlaveViewer : public wxFrame
{
    MainFrame* mainFrame;
    wxTimer timer;
    wxNotebook* notebook;
    GenericList* memoryList;

public:
    SlaveViewer(MainFrame* parent, MC68HC705C8* slave);
    ~SlaveViewer();

    void RefreshLoop(wxTimerEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // SLAVEVIEWER_HPP
