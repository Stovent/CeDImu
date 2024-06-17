#include "SCC68070.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

uint32_t SCC68070::GetEffectiveAddress(const uint8_t mode, const uint8_t reg, const uint8_t sizeInBytes, uint16_t& calcTime)
{
    switch(mode)
    {
    case 2:
        calcTime += sizeInBytes < 4 ? ITARIBW : ITARIL;
        return A(reg);

    case 3:
        calcTime += sizeInBytes < 4 ? ITARIWPoBW : ITARIWPoL;
        return AddressRegisterIndirectWithPostincrement(reg, sizeInBytes);

    case 4:
        calcTime += sizeInBytes < 4 ? ITARIWPrBW : ITARIWPrL;
        return AddressRegisterIndirectWithPredecrement(reg, sizeInBytes);

    case 5:
        calcTime += sizeInBytes < 4 ? ITARIWDBW : ITARIWDL;
        return AddressRegisterIndirectWithDisplacement(reg);

    case 6:
        calcTime += sizeInBytes < 4 ? ITARIWI8BW : ITARIWI8L;
        return AddressRegisterIndirectWithIndex8(reg);

    case 7:
        switch(reg)
        {
        case 0:
            calcTime += sizeInBytes < 4 ? ITASBW : ITASL;
            return AbsoluteShortAddressing();

        case 1:
            calcTime += sizeInBytes < 4 ? ITALBW : ITALL;
            return AbsoluteLongAddressing();

        case 2:
            calcTime += sizeInBytes < 4 ? ITPCIWDBW : ITPCIWDL;
            return ProgramCounterIndirectWithDisplacement();

        case 3:
            calcTime += sizeInBytes < 4 ? ITPCIWI8BW : ITPCIWI8L;
            return ProgramCounterIndirectWithIndex8();

        default:
            return UINT32_MAX;
        }

    default:
        return UINT32_MAX;
    }
}

int32_t SCC68070::GetIndexRegister(const uint16_t bew) const
{
    if(bew & 0x8000)
        if(bew & 0x0800)
            return A((bew & 0x7000) >> 12);
        else
            return signExtend<int16_t, int32_t>(A((bew & 0x7000) >> 12));
    else
        if(bew & 0x0800)
            return D[(bew & 0x7000) >> 12];
        else
            return signExtend<int16_t, int32_t>(D[(bew & 0x7000) >> 12]);
}

uint32_t SCC68070::AddressRegisterIndirectWithPostincrement(const uint8_t reg, const uint8_t sizeInByte)
{
    const uint32_t addr = A(reg);
    A(reg) += (reg == 7 && sizeInByte == 1) ? 2 : sizeInByte;
    return addr;
}

uint32_t SCC68070::AddressRegisterIndirectWithPredecrement(const uint8_t reg, const uint8_t sizeInByte)
{
    A(reg) -= (reg == 7 && sizeInByte == 1) ? 2 : sizeInByte;
    return A(reg);
}

uint32_t SCC68070::AddressRegisterIndirectWithDisplacement(const uint8_t reg)
{
    return A(reg) + signExtend<int16_t, int32_t>(GetNextWord());
}

uint32_t SCC68070::AddressRegisterIndirectWithIndex8(const uint8_t reg)
{
    const uint16_t bew = GetNextWord();
    return A(reg) + GetIndexRegister(bew) + signExtend<int8_t, int32_t>(bew & 0x00FF);
}

uint32_t SCC68070::ProgramCounterIndirectWithDisplacement()
{
    const uint32_t pc = PC;
    return pc + signExtend<int16_t, int32_t>(GetNextWord());
}

uint32_t SCC68070::ProgramCounterIndirectWithIndex8()
{
    const uint32_t pc = PC;
    const uint16_t bew = GetNextWord();
    return pc + GetIndexRegister(bew) + signExtend<int8_t, int32_t>(bew & 0x00FF);
}

uint32_t SCC68070::AbsoluteShortAddressing()
{
    return signExtend<int16_t, int32_t>(GetNextWord());
}

uint32_t SCC68070::AbsoluteLongAddressing()
{
    const uint16_t high = GetNextWord();
    const uint16_t low = GetNextWord();
    return high << 16 | low;
}

std::string SCC68070::DisassembleAddressingMode(const uint32_t extWordAddress, const uint8_t eamode, const uint8_t eareg, const uint8_t size, const bool hexImmediateData) const
{
    switch(eamode)
    {
    case 0:
        return "D" + std::to_string(eareg);

    case 1:
        return "A" + std::to_string(eareg);

    case 2:
        return "(A" + std::to_string(eareg) + ")";

    case 3:
        return "(A" + std::to_string(eareg) + ")+";

    case 4:
        return "-(A" + std::to_string(eareg) + ")";

    case 5:
        return "(" + std::to_string(as<int16_t>(cdi.GetWord(extWordAddress, NoFlags))) + ",A" + std::to_string(eareg) + ")";

    case 6:
    {
        const uint16_t bew = cdi.GetWord(extWordAddress, NoFlags);
        return "(" + std::to_string(as<int8_t>(bew)) + ",A" + std::to_string(eareg) + ((bew & 0x8000) ? ",A" : ",D") + std::to_string((bew & 0x7000) >> 12) + (bew & 0x0800 ? ".L" : ".W") + ")";
    }

    case 7:
        switch(eareg)
        {
        case 0:
            return "(0x" + toHex(cdi.GetWord(extWordAddress, NoFlags)) + ").W";

        case 1:
            return "(0x" + toHex(cdi.GetLong(extWordAddress, NoFlags)) + ").L";

        case 2:
            return "(" + std::to_string(as<int16_t>(cdi.GetWord(extWordAddress, NoFlags))) + ",PC)";

        case 3:
        {
            const uint16_t bew = cdi.GetWord(extWordAddress, NoFlags);
            return "(" + std::to_string(as<int8_t>(bew)) + ",PC," + ((bew & 0x8000) ? "A" : "D") + std::to_string((bew & 0x7000) >> 12) + (bew & 0x0800 ? ".L" : ".W") + ")";
        }

        case 4:
            if(hexImmediateData)
                return "#0x" + ((size == 1) ? toHex(cdi.GetWord(extWordAddress, NoFlags) & 0x00FF) : (size == 2) ? toHex(cdi.GetWord(extWordAddress, NoFlags)) : (size == 4) ? toHex(cdi.GetLong(extWordAddress, NoFlags)) : "Wrong size for immediate data");
            else
                return "#" + ((size == 1) ? std::to_string(cdi.GetWord(extWordAddress, NoFlags) & 0x00FF) : (size == 2) ? std::to_string(cdi.GetWord(extWordAddress, NoFlags)) : (size == 4) ? std::to_string(cdi.GetLong(extWordAddress, NoFlags)) : "Wrong size for immediate data");

        default:
            return "Wrong register for addressing mode 7";
        }

    default:
        return "Unknown addressing mode";
    }
}
