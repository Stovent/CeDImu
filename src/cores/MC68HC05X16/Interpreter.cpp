#include "MC68HC05X16.hpp"

#include <wx/msgdlg.h>

void MC68HC05X16::Interpreter()
{
    currentPC = PC;
    currentOpcode = GetNextByte();

    /*uint8_t executionTime = */(this->*ILUT[currentOpcode])();
    wxMessageBox((this->*DLUT[currentOpcode])());
}
