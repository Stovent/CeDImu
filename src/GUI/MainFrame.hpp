#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

class MainFrame;

#include <wx/frame.h>
#include <wx/timer.h>
#include <wx/menuitem.h>

#include "../CeDImu.hpp"
#include "GamePanel.hpp"
#include "DisassemblerFrame.hpp"
#include "RAMSearchFrame.hpp"

enum
{
    IDMainFrameOnOpenROM = wxID_HIGHEST + 1,
    IDMainFrameOnLoadBIOS,
    IDMainFrameOnCloseROM,
    IDMainFrameOnPause,
    IDMainFrameOnLimitFPS,
    IDMainFrameOnExecuteXInstructions,
    IDMainFrameOnRebootCore,
    IDMainFrameOnExportFiles,
    IDMainFrameOnExportAudio,
    IDMainFrameOnExportVideo,
    IDMainFrameOnVDSCViewer,
    IDMainFrameOnDisassembler,
    IDMainFrameOnRAMSearch,
    IDMainFrameOnSettings,
    IDMainFrameOnAbout,

    IDDisassemblerOnClose,
    IDDisassemblerpc, IDDisassemblersr,

    IDRAMSearchListCheckMisaligned,
    IDRAMSearchListSigned,
    IDRAMSearchListUnsigned,
    IDRAMSearchListHexadecimal,
    IDRAMSearchListByte1,
    IDRAMSearchListByte2,
    IDRAMSearchListByte4,
};

class MainFrame : public wxFrame
{
public:
    CeDImu* app;
    GamePanel* gamePanel;
    wxMenuItem* pauseItem;
    wxMenuItem* limitFPSItem;

    MainFrame(CeDImu* appp, const wxString& title, const wxPoint& pos, const wxSize& size);
    DisassemblerFrame* disassemblerFrame;
    RAMSearchFrame* ramSearchFrame;

    void Pause();
    void RefreshTitle(wxTimerEvent& event);

private:
    wxTimer renderTimer;
    uint32_t oldFrameCount;
    uint64_t oldCycleCount;

    void CreateMenuBar();
    void OnOpenROM(wxCommandEvent& event);
    void OnOpenBinary(wxCommandEvent& event);
    void OnLoadBIOS(wxCommandEvent& event);
    void OnCloseROM(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);

    void OnPause(wxCommandEvent& event);
    void OnLimitFPS(wxCommandEvent& event);
    void OnExecuteXInstructions(wxCommandEvent& event);
    void OnRebootCore(wxCommandEvent& event);

    void OnVDSCViewer(wxCommandEvent& event);
    void OnDisassembler(wxCommandEvent& event);
    void OnRAMSearch(wxCommandEvent& event);

    void OnExportFiles(wxCommandEvent& event);
    void OnExportAudio(wxCommandEvent& event);
    void OnExportVideo(wxCommandEvent& event);

    void OnSettings(wxCommandEvent& event);

    void OnAbout(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // MAINFRAME_HPP
