#include "SlaveViewer.hpp"
#include "enums.hpp"
#include "MainFrame.hpp"
#include "../utils.hpp"

#include <algorithm>
#include <iterator>
#include <sstream>

#include <wx/dcclient.h>

wxBEGIN_EVENT_TABLE(SlaveViewer, wxFrame)
    EVT_TIMER(IDVDSCViewerTimer, SlaveViewer::RefreshLoop)
wxEND_EVENT_TABLE()

SlaveViewer::SlaveViewer(MainFrame* parent, Board* board) : wxFrame(parent, wxID_ANY, "Slave Viewer"), timer(this, IDSlaveViewerTimer)
{
    mainFrame = parent;
    this->board = board;
    timer.Start(16);
}

SlaveViewer::~SlaveViewer()
{
    mainFrame->slaveViewer = nullptr;
}

void SlaveViewer::RefreshLoop(wxTimerEvent& event)
{
}
