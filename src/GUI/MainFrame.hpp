#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

class MainFrame;

#include <wx/frame.h>
#include <wx/menuitem.h>

#include "../CeDImu.hpp"
#include "GamePanel.hpp"
#include "DisassemblerFrame.hpp"
#include "RAMWatchFrame.hpp"

enum
{
    IDOnOpenROM = wxID_HIGHEST + 1,
    IDOnOpenBinary,
    IDOnLoadBIOS,
    IDOnCloseROM,
    IDOnPause,
    IDOnRebootCore,
    IDOnExportFiles,
    IDOnExportAudio,
    IDOnDisassembler,
    IDOnRAMWatch,
    IDOnAbout,

    IDDisassemblerOnClose,

    IDDisassemblerpc, IDDisassemblersr,
};

class MainFrame : public wxFrame
{
public:
    wxMenuItem* pause;

    MainFrame(CeDImu* appp, const wxString& title, const wxPoint& pos, const wxSize& size);
    DisassemblerFrame* disassemblerFrame;
    RAMWatchFrame* ramWatchFrame;

    void OnPause();

private:
    CeDImu* app;
    GamePanel* gamePanel;

    void CreateMenuBar();
    void OnOpenROM(wxCommandEvent& event);
    void OnOpenBinary(wxCommandEvent& event);
    void OnLoadBIOS(wxCommandEvent& event);
    void OnCloseROM(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);

    void OnPause(wxCommandEvent& event);
    void OnRebootCore(wxCommandEvent& event);

    void OnDisassembler(wxCommandEvent& event);
    void OnRAMWatch(wxCommandEvent& event);

    void OnExportFiles(wxCommandEvent& event);
    void OnExportAudio(wxCommandEvent& event);

    void OnAbout(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // MAINFRAME_HPP
