#ifndef GENERICLIST_HPP
#define GENERICLIST_HPP

#include <wx/listctrl.h>

#include <functional>

class GenericList : public wxListCtrl
{
    std::function<std::string(long, long)> OnGetItem;

public:
    GenericList(wxWindow* parent, std::function<void(wxListCtrl*)> builder, std::function<std::string(long, long)> getter) :
        wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES | wxLC_VRULES),
        OnGetItem(getter)
    {
        builder(this);
    }

    virtual wxString OnGetItemText(long item, long column) const override
    {
        return OnGetItem(item, column);
    }
};

#endif // GENERICLIST_HPP

