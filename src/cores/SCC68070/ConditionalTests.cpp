#include "SCC68070.hpp"
// M68000RMP p.90 Table 3-19
bool SCC68070::T() const
{
    return true;
}

bool SCC68070::F() const
{
    return false;
}

bool SCC68070::HI() const
{
    return !GetC() && !GetZ();
}

bool SCC68070::LS() const
{
    return GetC() || GetZ();
}

bool SCC68070::CC() const
{
    return !GetC();
}

bool SCC68070::CS() const
{
    return GetC();
}

bool SCC68070::NE() const
{
    return !GetZ();
}

bool SCC68070::EQ() const
{
    return GetZ();
}

bool SCC68070::VC() const
{
    return !GetV();
}

bool SCC68070::VS() const
{
    return GetV();
}

bool SCC68070::PL() const
{
    return !GetN();
}

bool SCC68070::MI() const
{
    return GetN();
}

bool SCC68070::GE() const
{
    return (GetN() && GetV()) || (!GetN() && !GetV());
}

bool SCC68070::LT() const
{
    return (GetN() && !GetV()) || (!GetN() && GetV());
}

bool SCC68070::GT() const
{
    return (GetN() && GetV() && !GetZ()) || (!GetN() && !GetV() && !GetZ());
}

bool SCC68070::LE() const
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
