#include "SCC68070.hpp"
#include "../../utils.hpp"

uint8_t SCC68070::GetByte(const uint8_t& mode, const uint8_t& reg, uint16_t& calcTime)
{
    if(mode == 0)
    {
        lastAddress = 0;
        return D[reg] & 0x000000FF;
    }
    else if(mode == 1)
    {
        lastAddress = 0;
        return A[reg] & 0x000000FF;
    }
    else if(mode == 2)
    {
        lastAddress = A[reg];
        calcTime += ITARIBW;
    }
    else if(mode == 3)
    {
        lastAddress = ARIWPo(reg, 1);
        calcTime += ITARIWPoBW;
    }
    else if(mode == 4)
    {
        lastAddress = ARIWPr(reg, 1);
        calcTime += ITARIWPrBW;
    }
    else if(mode == 5)
    {
        lastAddress = ARIWD(reg);
        calcTime += ITARIWDBW;
    }
    else if(mode == 6)
    {
        lastAddress = ARIWI8(reg);
        calcTime += ITARIWI8BW;
    }
    else if(mode == 7)
    {
        if(reg == 0)
        {
            lastAddress = ASA();
            calcTime += ITASBW;
        }
        else if(reg == 1)
        {
            lastAddress = ALA();
            calcTime += ITALBW;
        }
        else if(reg == 2)
        {
            lastAddress = PCIWD();
            calcTime += ITPCIWDBW;
        }
        else if(reg == 3)
        {
            lastAddress = PCIWI8();
            calcTime += ITPCIWI8BW;
        }
        else if(reg == 4)
        {
            calcTime += ITIBW;
            return GetNextWord() & 0x00FF;
        }
    }
    return GetByte(lastAddress);
}

uint16_t SCC68070::GetWord(const uint8_t& mode, const uint8_t& reg, uint16_t& calcTime)
{
    if(mode == 0)
    {
        lastAddress = 0;
        return D[reg] & 0x0000FFFF;
    }
    else if(mode == 1)
    {
        lastAddress = 0;
        return A[reg] & 0x0000FFFF;
    }
    else if(mode == 2)
    {
        lastAddress = A[reg];
        calcTime += ITARIBW;
    }
    else if(mode == 3)
    {
        lastAddress = ARIWPo(reg, 2);
        calcTime += ITARIWPoBW;
    }
    else if(mode == 4)
    {
        lastAddress = ARIWPr(reg, 2);
        calcTime += ITARIWPrBW;
    }
    else if(mode == 5)
    {
        lastAddress = ARIWD(reg);
        calcTime += ITARIWDBW;
    }
    else if(mode == 6)
    {
        lastAddress = ARIWI8(reg);
        calcTime += ITARIWI8BW;
    }
    else if(mode == 7)
    {
        if(reg == 0)
        {
            lastAddress = ASA();
            calcTime += ITASBW;
        }
        else if(reg == 1)
        {
            lastAddress = ALA();
            calcTime += ITALBW;
        }
        else if(reg == 2)
        {
            lastAddress = PCIWD();
            calcTime += ITPCIWDBW;
        }
        else if(reg == 3)
        {
            lastAddress = PCIWI8();
            calcTime += ITPCIWI8BW;
        }
        else if(reg == 4)
        {
            calcTime += ITIBW;
            return GetNextWord();
        }
    }
    return GetWord(lastAddress);
}

uint32_t SCC68070::GetLong(const uint8_t& mode, const uint8_t& reg, uint16_t& calcTime)
{
    if(mode == 0)
    {
        lastAddress = 0;
        return D[reg];
    }
    else if(mode == 1)
    {
        lastAddress = 0;
        return A[reg];
    }
    else if(mode == 2)
    {
        lastAddress = A[reg];
        calcTime += ITARIL;
    }
    else if(mode == 3)
    {
        lastAddress = ARIWPo(reg, 4);
        calcTime += ITARIWPoL;
    }
    else if(mode == 4)
    {
        lastAddress = ARIWPr(reg, 4);
        calcTime += ITARIWPrL;
    }
    else if(mode == 5)
    {
        lastAddress = ARIWD(reg);
        calcTime += ITARIWDL;
    }
    else if(mode == 6)
    {
        lastAddress = ARIWI8(reg);
        calcTime += ITARIWI8L;
    }
    else if(mode == 7)
    {
        if(reg == 0)
        {
            lastAddress = ASA();
            calcTime += ITASL;
        }
        else if(reg == 1)
        {
            lastAddress = ALA();
            calcTime += ITALL;
        }
        else if(reg == 2)
        {
            lastAddress = PCIWD();
            calcTime += ITPCIWDL;
        }
        else if(reg == 3)
        {
            lastAddress = PCIWI8();
            calcTime += ITPCIWI8L;
        }
        else if(reg == 4)
        {
            calcTime += ITIL;
            return GetNextWord() << 16 | GetNextWord();
        }
    }
    return GetLong(lastAddress);
}

void SCC68070::SetByte(const uint8_t& mode, const uint8_t& reg, uint16_t& calcTime, const uint8_t& data)
{
    if(mode == 0)
    {
        lastAddress = 0;
        D[reg] &= 0xFFFFFF00;
        D[reg] |= data;
        return;
    }
    else if(mode == 2)
    {
        lastAddress = A[reg];
        calcTime += ITARIBW;
    }
    else if(mode == 3)
    {
        lastAddress = ARIWPo(reg, 1);
        calcTime += ITARIWPoBW;
    }
    else if(mode == 4)
    {
        lastAddress = ARIWPr(reg, 1);
        calcTime += ITARIWPrBW;
    }
    else if(mode == 5)
    {
        lastAddress = ARIWD(reg);
        calcTime += ITARIWDBW;
    }
    else if(mode == 6)
    {
        lastAddress = ARIWI8(reg);
        calcTime += ITARIWI8BW;
    }
    else if(mode == 7)
    {
        if(reg == 0)
        {
            lastAddress = ASA();
            calcTime += ITASBW;
        }
        else if(reg == 1)
        {
            lastAddress = ALA();
            calcTime += ITALBW;
        }
        else if(reg == 2)
        {
            lastAddress = PCIWD();
            calcTime += ITPCIWDBW;
        }
        else if(reg == 3)
        {
            lastAddress = PCIWI8();
            calcTime += ITPCIWI8BW;
        }
    }
    SetByte(lastAddress, data);
}

void SCC68070::SetWord(const uint8_t& mode, const uint8_t& reg, uint16_t& calcTime, const uint16_t& data)
{
    if(mode == 0)
    {
        lastAddress = 0;
        D[reg] &= 0xFFFF0000;
        D[reg] |= data;
        return;
    }
    else if(mode == 1)
    {
        lastAddress = 0;
        A[reg] = signExtend16(data);
        return;
    }
    else if(mode == 2)
    {
        lastAddress = A[reg];
        calcTime += ITARIBW;
    }
    else if(mode == 3)
    {
        lastAddress = ARIWPo(reg, 2);
        calcTime += ITARIWPoBW;
    }
    else if(mode == 4)
    {
        lastAddress = ARIWPr(reg, 2);
        calcTime += ITARIWPrBW;
    }
    else if(mode == 5)
    {
        lastAddress = ARIWD(reg);
        calcTime += ITARIWDBW;
    }
    else if(mode == 6)
    {
        lastAddress = ARIWI8(reg);
        calcTime += ITARIWI8BW;
    }
    else if(mode == 7)
    {
        if(reg == 0)
        {
            lastAddress = ASA();
            calcTime += ITASBW;
        }
        else if(reg == 1)
        {
            lastAddress = ALA();
            calcTime += ITALBW;
        }
        else if(reg == 2)
        {
            lastAddress = PCIWD();
            calcTime += ITPCIWDBW;
        }
        else if(reg == 3)
        {
            lastAddress = PCIWI8();
            calcTime += ITPCIWI8BW;
        }
    }
    SetWord(lastAddress, data);
}

void SCC68070::SetLong(const uint8_t& mode, const uint8_t& reg, uint16_t& calcTime, const uint32_t& data)
{
    if(mode == 0)
    {
        lastAddress = 0;
        D[reg] = data;
        return;
    }
    else if(mode == 1)
    {
        lastAddress = 0;
        A[reg] = data;
        return;
    }
    else if(mode == 2)
    {
        lastAddress = A[reg];
        calcTime += ITARIL;
    }
    else if(mode == 3)
    {
        lastAddress = ARIWPo(reg, 4);
        calcTime += ITARIWPoL;
    }
    else if(mode == 4)
    {
        lastAddress = ARIWPr(reg, 4);
        calcTime += ITARIWPrL;
    }
    else if(mode == 5)
    {
        lastAddress = ARIWD(reg);
        calcTime += ITARIWDL;
    }
    else if(mode == 6)
    {
        lastAddress = ARIWI8(reg);
        calcTime += ITARIWI8L;
    }
    else if(mode == 7)
    {
        if(reg == 0)
        {
            lastAddress = ASA();
            calcTime += ITASL;
        }
        else if(reg == 1)
        {
            lastAddress = ALA();
            calcTime += ITALL;
        }
        else if(reg == 2)
        {
            lastAddress = PCIWD();
            calcTime += ITPCIWDL;
        }
        else if(reg == 3)
        {
            lastAddress = PCIWI8();
            calcTime += ITPCIWI8L;
        }
    }
    SetLong(lastAddress, data);
}

uint8_t SCC68070::GetByte(const uint32_t& addr)
{
    out << std::hex << currentPC << "\tGet byte: 0x" << addr << std::endl;
    if(addr < 0x80000000 || addr >= 0xC0000000)
        return vdsc.GetByte(addr);
    else if(addr >= 0x80000000 && addr < 0x80008080)
        return internal[addr-INTERNAL];
    else
        return 0;
}

uint16_t SCC68070::GetWord(const uint32_t& addr)
{
    out << std::hex << currentPC << "\tGet word: 0x" << addr << std::endl;
    if(addr < 0x80000000 || addr >= 0xC0000000)
        return vdsc.GetWord(addr);
    else
        return 0;
}

uint32_t SCC68070::GetLong(const uint32_t& addr)
{
    out << std::hex << currentPC << "\tGet long: 0x" << addr << std::endl;
    if(addr < 0x80000000 || addr >= 0xC0000000)
        return vdsc.GetLong(addr);
    else
        return 0;
}

void SCC68070::SetByte(const uint32_t& addr, const uint8_t& data)
{
    out << std::hex << currentPC << "\tSet byte: 0x" << addr << " value: " << std::to_string(data) << std::endl;
    if(addr < 0x80000000 || addr >= 0xC0000000)
        vdsc.SetByte(addr, data);
}

void SCC68070::SetWord(const uint32_t& addr, const uint16_t& data)
{
    out << std::hex << currentPC << "\tSet word: 0x" << addr << " value: " << std::to_string(data) << std::endl;
    if(addr < 0x80000000 || addr >= 0xC0000000)
        vdsc.SetWord(addr, data);
}

void SCC68070::SetLong(const uint32_t& addr, const uint32_t& data)
{
    out << std::hex << currentPC << "\tSet Long: 0x" << addr << " value: " << std::to_string(data) << std::endl;
    if(addr < 0x80000000 || addr >= 0xC0000000)
        vdsc.SetLong(addr, data);
}
