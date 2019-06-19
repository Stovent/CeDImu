#ifndef GENERIC_FRAME_HPP
#define GENERIC_FRAME_HPP

class GenericFrame;

#include <wx/frame.h>
#include <wx/sizer.h>

class GenericFrame : public wxFrame
{
    wxFrame* parent;

public:
    GenericFrame(wxFrame* parent, wxString title, wxPoint pos, wxSize size) : wxFrame(parent, wxID_ANY, title, pos, size), parent(parent){}
};

#endif // GENERIC_FRAME_HPP
