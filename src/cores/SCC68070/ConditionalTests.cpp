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

std::string SCC68070::DisassembleConditionalCode(uint8_t cc)
{
    std::string ret;
    switch(cc)
    {
    case 0: ret = "T"; break;
    case 1: ret = "F"; break;
    case 2: ret = "HI"; break;
    case 3: ret = "LS"; break;
    case 4: ret = "CC"; break;
    case 5: ret = "CS"; break;
    case 6: ret = "NE"; break;
    case 7: ret = "EQ"; break;
    case 8: ret = "VC"; break;
    case 9: ret = "VS"; break;
    case 10: ret = "PL"; break;
    case 11: ret = "MI"; break;
    case 12: ret = "GE"; break;
    case 13: ret = "LT"; break;
    case 14: ret = "GT"; break;
    case 15: ret = "LE"; break;
    }
    return ret;
}
