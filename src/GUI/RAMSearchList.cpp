#include "RAMSearchList.hpp"
#include "../CeDImu.hpp"
#include "../CDI/common/utils.hpp"

#define GET_ARRAY16(array, index) ((uint16_t)array[(index)] << 8 | array[(index)+1])
#define GET_ARRAY32(array, index) ((uint32_t)array[(index)] << 24 | array[(index)+1] << 16 | array[(index)+2] << 8 | array[(index)+3])

RAMSearchList::RAMSearchList(RAMSearchFrame* parent) : wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES | wxLC_VRULES)
{
    ramSearchFrame = parent;

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

    EnableAlternateRowColours();
    SetItemCount(ramSearchFrame->board.GetAllocatedMemory());
}

wxString RAMSearchList::OnGetItemText(long item, long column) const
{
    RAMBank bank1 = ramSearchFrame->board.GetRAMBank1();
    RAMBank bank2 = ramSearchFrame->board.GetRAMBank2();
    const uint8_t* memory;
    uint32_t base;

    if(!ramSearchFrame->checkMisaligned->GetValue())
    {
        if(ramSearchFrame->byte2->GetValue())
            item *= 2;
        else if(ramSearchFrame->byte4->GetValue())
            item *= 4;
    }

    if(item < (long)bank1.size)
    {
        memory = bank1.data;
        base = bank1.base;
    }
    else
    {
        item -= bank1.size;
        if(item < (long)bank2.size)
        {
            memory = bank2.data;
            base = bank2.base;
        }
        else
            return "-1";
    }

    if(column == 1)
        if(ramSearchFrame->byte1->GetValue())
            if(ramSearchFrame->signed_->GetValue())
                return std::to_string((int8_t)memory[item]);
            else if(ramSearchFrame->unsigned_->GetValue())
                return std::to_string(memory[item]);
            else
                return toHex(memory[item]);

        else if(ramSearchFrame->byte2->GetValue())
            if(ramSearchFrame->signed_->GetValue())
                return std::to_string((int16_t)GET_ARRAY16(memory, item));
            else if(ramSearchFrame->unsigned_->GetValue())
                return std::to_string(GET_ARRAY16(memory, item));
            else
                return toHex(GET_ARRAY16(memory, item));

        else
            if(ramSearchFrame->signed_->GetValue())
                return std::to_string((int32_t)GET_ARRAY32(memory, item));
            else if(ramSearchFrame->unsigned_->GetValue())
                return std::to_string(GET_ARRAY32(memory, item));
            else
                return toHex(GET_ARRAY32(memory, item));
    else
        return toHex(base + item);
}
