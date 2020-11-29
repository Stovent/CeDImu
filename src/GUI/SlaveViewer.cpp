#include "SlaveViewer.hpp"
#include "enums.hpp"
#include "MainFrame.hpp"
#include "../cores/MC68HC705C8/MC68HC705C8.hpp"

#include <wx/dcclient.h>

#include <algorithm>
#include <iterator>
#include <sstream>

wxBEGIN_EVENT_TABLE(SlaveViewer, wxFrame)
    EVT_TIMER(IDSlaveViewerTimer, SlaveViewer::RefreshLoop)
wxEND_EVENT_TABLE()

SlaveViewer::SlaveViewer(MainFrame* parent, MC68HC705C8* slave) : wxFrame(parent, wxID_ANY, "Slave Viewer"), timer(this, IDSlaveViewerTimer)
{
    mainFrame = parent;
    timer.Start(16);

    memoryList = new GenericList(this, slave->GetMemory(), SLAVE_MEMORY_SIZE);
}

SlaveViewer::~SlaveViewer()
{
    mainFrame->slaveViewer = nullptr;
}

void SlaveViewer::RefreshLoop(wxTimerEvent& event)
{
    memoryList->Refresh();
}
