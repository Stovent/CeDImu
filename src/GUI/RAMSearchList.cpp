#include "RAMSearchList.hpp"

RAMSearchList::RAMSearchList(RAMSearchFrame* parent) : wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES | wxLC_VRULES)
{
    mainFrame = parent;

    wxListItem addressCol;
    addressCol.SetId(0);
    addressCol.SetText("Address");
    addressCol.SetWidth(100);
    InsertColumn(0, addressCol);

    wxListItem valueCol;
    valueCol.SetId(1);
    valueCol.SetText("Value");
    valueCol.SetWidth(100);
    InsertColumn(1, valueCol);

    SetItemCount(1024*1024*5);
}

wxString RAMSearchList::OnGetItemText(long item, long column) const
{
    if(column == 1)
        return std::to_string(mainFrame->vdsc->GetByte(item));
    else
        return toHex(item);
}
