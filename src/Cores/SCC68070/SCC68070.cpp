#include "SCC68070.hpp"

SCC68070::SCC68070()
{
    Execute = Interpreter;
}

void SCC68070::Run()
{
    while(run)
    {
        (this->*Execute)();
    }
}
