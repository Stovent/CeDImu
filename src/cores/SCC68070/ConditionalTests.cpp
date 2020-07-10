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
    return (GetN() && !GetV()) || (!GetN() && GetV());
}

bool SCC68070::GT()
{
    return (GetN() && GetV() && !GetZ()) || (!GetN() && !GetV() && !GetZ());
}

bool SCC68070::LE()
{
    return GetZ() || (GetN() && !GetV()) || (!GetN() && GetV());
}

std::string SCC68070::DisassembleConditionalCode(const uint8_t cc) const
{
    switch(cc)
    {
    case 0: return "T";
    case 1: return "F";
    case 2: return "HI";
    case 3: return "LS";
    case 4: return "CC";
    case 5: return "CS";
    case 6: return "NE";
    case 7: return "EQ";
    case 8: return "VC";
    case 9: return "VS";
    case 10: return "PL";
    case 11: return "MI";
    case 12: return "GE";
    case 13: return "LT";
    case 14: return "GT";
    case 15: return "LE";
    default: return "Unknown";
    }
}
