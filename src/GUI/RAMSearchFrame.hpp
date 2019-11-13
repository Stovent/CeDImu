#ifndef RAMSEARCHFRAME_HPP
#define RAMSEARCHFRAME_HPP

class RAMSearchFrame;

#include <wx/frame.h>
#include <wx/timer.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>

#include "MainFrame.hpp"
#include "RAMSearchList.hpp"
#include "../Cores/VDSC.hpp"

class RAMSearchFrame : public wxFrame
{
public:
    VDSC* vdsc;
    MainFrame* mainFrame;
    RAMSearchList* ramSearchList;
    wxTimer* renderTimer;

    wxCheckBox* checkMisaligned;
    wxRadioButton* signed_;
    wxRadioButton* unsigned_;
    wxRadioButton* hexadecimal_;
    wxRadioButton* byte1;
    wxRadioButton* byte2;
    wxRadioButton* byte4;

    RAMSearchFrame(VDSC* vds, MainFrame* parent, const wxPoint& pos, const wxSize& size);
    ~RAMSearchFrame();

    void OnClose(wxCloseEvent& event);

    void PaintEvent();
    void RefreshLoop(wxTimerEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // RAMSEARCHFRAME_HPP
