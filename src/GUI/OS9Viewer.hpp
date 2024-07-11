#ifndef GUI_OS9VIEWER_HPP
#define GUI_OS9VIEWER_HPP

class MainFrame;
#include "../CeDImu.hpp"

#include <wx/aui/auibook.h>
#include <wx/frame.h>

class OS9Viewer : public wxFrame
{
    CeDImu& m_cedimu;
    MainFrame* m_mainFrame;
    wxAuiNotebook* m_auiNotebook;

public:
    OS9Viewer() = delete;
    OS9Viewer(MainFrame* mainFrame, CeDImu& cedimu);
    ~OS9Viewer();
};

#endif // GUI_OS9VIEWER_HPP
