#include "GenericList.hpp"
#include "../CDI/common/utils.hpp"

GenericList::GenericList(wxWindow* parent, uint8_t* memory, long size) : wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES | wxLC_VRULES)
{
    this->memory = memory;
    this->size = size;

    EnableAlternateRowColours();
    SetItemCount(size);

    wxListItem addressCol;
    addressCol.SetId(0);
    addressCol.SetText("Address");
    addressCol.SetWidth(60);
    InsertColumn(0, addressCol);

    wxListItem hexCol;
    hexCol.SetId(1);
    hexCol.SetText("Value hex");
    hexCol.SetWidth(70);
    InsertColumn(1, hexCol);

    wxListItem signedCol;
    signedCol.SetId(2);
    signedCol.SetText("Value signed");
    signedCol.SetWidth(85);
    InsertColumn(2, signedCol);

    wxListItem unsignedCol;
    unsignedCol.SetId(3);
    unsignedCol.SetText("Value unsigned");
    unsignedCol.SetWidth(95);
    InsertColumn(3, unsignedCol);
}

wxString GenericList::OnGetItemText(long item, long column) const
{
    if(column == 0)
        return toHex(item);
    else if(column == 1)
        return toHex(memory[item]);
    else if(column == 2)
        return std::to_string((int8_t)memory[item]);
    else
        return std::to_string(memory[item]);
}
