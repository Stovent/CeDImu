#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

class MainFrame;

#include <wx/frame.h>

#include "../CeDImu.hpp"
#include "GamePanel.hpp"
#include "DisassemblerFrame.hpp"

enum
{
    IDOnOpenROM = wxID_LAST + 1,
    IDOnCloseROM,
    IDOnAbout,
    IDOnDisassembler,

    IDDisassemblerOnClose,

    IDDisassemblerd0, IDDisassemblera0,
    IDDisassemblerd1, IDDisassemblera1,
    IDDisassemblerd2, IDDisassemblera2,
    IDDisassemblerd3, IDDisassemblera3,
    IDDisassemblerd4, IDDisassemblera4,
    IDDisassemblerd5, IDDisassemblera5,
    IDDisassemblerd6, IDDisassemblera6,
    IDDisassemblerd7, IDDisassemblera7,
    IDDisassemblerdpc, IDDisassemblerasr,
};

class MainFrame : public wxFrame
{
public:
    MainFrame(CeDImu* appp, const wxString& title, const wxPoint& pos, const wxSize& size);
    DisassemblerFrame* disassemblerFrame;

private:
    CeDImu* app;
    GamePanel* gamePanel;


    void CreateMenuBar();
    void OnOpenROM(wxCommandEvent& event);
    void OnCloseROM(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);

    void OnDisassembler(wxCommandEvent& event);

    void OnAbout(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // MAINFRAME_HPP
