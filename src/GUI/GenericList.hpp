#ifndef GENERICLIST_HPP
#define GENERICLIST_HPP

#include <wx/listctrl.h>

class GenericList : public wxListCtrl
{
    uint8_t* memory;
    long size;

public:
    GenericList(wxWindow* parent, uint8_t* memory, long size);

    virtual wxString OnGetItemText(long item, long column) const override;
};

#endif // GENERICLIST_HPP

