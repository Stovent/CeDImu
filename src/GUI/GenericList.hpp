#ifndef GUI_GENERICLIST_HPP
#define GUI_GENERICLIST_HPP

#include <wx/listctrl.h>

#include <functional>

class GenericList : public wxListCtrl
{
    std::function<wxString(long, long)> m_OnGetItem;

public:
    GenericList(wxWindow* parent, std::function<void(wxListCtrl*)> builder, std::function<wxString(long, long)> getter) :
        wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES | wxLC_VRULES),
        m_OnGetItem(getter)
    {
        builder(this);
    }

    virtual wxString OnGetItemText(long item, long column) const override
    {
        return m_OnGetItem(item, column);
    }
};

#endif // GUI_GENERICLIST_HPP
