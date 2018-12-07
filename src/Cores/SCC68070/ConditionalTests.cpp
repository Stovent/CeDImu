#include "SCC68070.hpp"
// M68000RMP p.90 Table 3-19
bool SCC68070::T()
{
    return true;
}

bool SCC68070::F()
{
    return false;
}

bool SCC68070::HI()
{
    return !GetC() && !GetZ();
}

bool SCC68070::LS()
{
    return GetC() || GetZ();
}

bool SCC68070::CC()
{
    return !GetC();
}

bool SCC68070::CS()
{
    return GetC();
}

bool SCC68070::NE()
{
    return !GetZ();
}

bool SCC68070::EQ()
{
    return GetZ();
}

bool SCC68070::VC()
{
    return !GetV();
}

bool SCC68070::VS()
{
    return GetV();
}

bool SCC68070::PL()
{
    return !GetN();
}

bool SCC68070::MI()
{
    return GetN();
}

bool SCC68070::GE()
{
    return (GetN() && GetV()) || (!GetN() && !GetV());
}

bool SCC68070::LT()
{
    return (GetN() && !GetV()) || (GetN() && GetV());
}

bool SCC68070::GT()
{
    return (GetN() && GetV() && !GetZ()) || (!GetN() && !GetV() && !GetZ());
}

bool SCC68070::LE()
{
    return GetZ() || (GetN() && !GetV()) || (!GetN() && GetV());
}
