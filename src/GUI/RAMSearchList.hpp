#ifndef RAMSEARCHLIST_HPP
#define RAMSEARCHLIST_HPP

class RAMSearchList;

#include <wx/listctrl.h>

#include "RAMSearchFrame.hpp"

class RAMSearchList : public wxListCtrl
{
public:
    RAMSearchFrame* ramSearchFrame;

    RAMSearchList(RAMSearchFrame* parent);
    virtual wxString OnGetItemText(long item, long column) const override;
};

#endif // RAMSEARCHLIST_HPP
