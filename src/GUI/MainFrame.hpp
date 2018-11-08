#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

class MainFrame;

#include <wx/wx.h>

#include "../CeDImu.hpp"
#include "GamePanel.hpp"

enum
{
    IDFileOpenROM = wxID_LAST + 1,
    IDFileCloseROM,
    IDFileOpenMovie,
    IDFileSaveStateAs,
    IDFileLoadStateFrom,

    IDToolsDisassembler,
    IDToolsCPUViewer,
    IDToolsRAMSearch,
    IDToolsRAMWatch,

    IDCdiDiskLabel,
    IDCdiExport,
    IDCdiExportDiskSectorsInfo,
    IDCdiExportFiles,
    IDCdiExportAudio,
    IDCdiExportVideo,

    IDHelpWebsite,
    IDHelpAbout,
};

class MainFrame : public wxFrame
{
public:
    MainFrame(CeDImu* appp, const wxString& title, const wxPoint& pos, const wxSize& size);
    void CreateMenuBar();

private:
    CeDImu* app;
    GamePanel* gamePanel;

    void OnAbout(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnOpenMovie(wxCommandEvent& event);
    void OnOpenROM(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // MAINFRAME_HPP
