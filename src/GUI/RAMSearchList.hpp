#ifndef RAMSEARCHLIST_HPP
#define RAMSEARCHLIST_HPP

class RAMSearchList;

#include "RAMSearchFrame.hpp"

#include <wx/listctrl.h>

class RAMSearchList : public wxListCtrl
{
public:
    RAMSearchFrame* ramSearchFrame;

    RAMSearchList(RAMSearchFrame* parent);

    virtual wxString OnGetItemText(long item, long column) const override;
};

#endif // RAMSEARCHLIST_HPP
