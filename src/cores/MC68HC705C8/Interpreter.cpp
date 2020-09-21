#include "MC68HC705C8.hpp"

#include <wx/msgdlg.h>

void MC68HC705C8::Interpreter()
{
    currentPC = PC;
    currentOpcode = GetNextByte();

    /*uint8_t executionTime = */(this->*ILUT[currentOpcode])();
    wxMessageBox((this->*DLUT[currentOpcode])());
}
