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
    if(addr < 0x80000000 || addr >= 0xC0000000)
        return vdsc->GetByte(addr);
    else if(addr >= 0x80000000 && addr < 0x80008080)
    {
        if(GetS())
        {
            if(addr == 0x8000201B)
            {
                if(UART_IN.empty())
                    internal[addr-INTERNAL] = INT8_MIN;
                else
                {
                    internal[addr-INTERNAL] = UART_IN.front();
                    UART_IN.pop();
                }
#ifdef DEBUG
                out << "URHR 0x" << std::hex << currentPC << " value #" << (uint32_t)internal[addr-INTERNAL] << std::endl;
#endif // DEBUG
            }
            return internal[addr-INTERNAL];
        }
        else
            return vdsc->GetByte(addr);
    }
#ifdef DEBUG
    else
        out << "OUT OF RANGE ";
    out << std::hex << currentPC << "\tGet byte: 0x" << addr << std::endl;
#endif // DEBUG
    return 0;
}

uint16_t SCC68070::GetWord(const uint32_t& addr)
{
    if(addr < 0x80000000 || addr >= 0xC0000000)
        return vdsc->GetWord(addr);
#ifdef DEBUG
    else
        out << "OUT OF RANGE ";
    out << std::hex << currentPC << "\tGet word: 0x" << addr << std::endl;
#endif // DEBUG
    return 0;
}

uint32_t SCC68070::GetLong(const uint32_t& addr)
{
    if(addr < 0x80000000 || addr >= 0xC0000000)
        return vdsc->GetLong(addr);
#ifdef DEBUG
    else
        out << "OUT OF RANGE ";
    out << std::hex << currentPC << "\tGet long: 0x" << addr << std::endl;
#endif // DEBUG
    return 0;
}

void SCC68070::SetByte(const uint32_t& addr, const uint8_t& data)
{
    if(addr < 0x80000000 || addr >= 0xC0000000)
        vdsc->SetByte(addr, data);
    else if(addr >= 0x80000000 || addr < 0x80008080)
    {
        internal[addr-INTERNAL] = data;
        if(addr == 0x80002017) // UART Command Register
        {
            switch(data)
            {
            case 2: // reset receiver
                internal[URHR] = 0;
                break;
            case 3: // reset transmitter
                internal[UTHR] = 0;
                break;
            case 4: // Reset error status
                internal[USR] &= 0x0F;
                break;
            }
        }
        else if(addr == 0x80002019) // UART Transmit Holding Register
        {
            UART_OUT.push(data);
            internal[USR] |= 0x08; // set TXEMT bit
#ifdef DEBUG
            uart_out.write((char*)&data, 1);
#endif // DEBUG
        }
    }
#ifdef DEBUG
    else
        out << "OUT OF RANGE ";
    out << std::hex << currentPC << "\tSet byte: 0x" << addr << " value: " << std::to_string(data) << std::endl;
#endif // DEBUG
}

void SCC68070::SetWord(const uint32_t& addr, const uint16_t& data)
{
    if(addr < 0x80000000 || addr >= 0xC0000000)
        vdsc->SetWord(addr, data);
#ifdef DEBUG
    else
        out << "OUT OF RANGE ";
    out << std::hex << currentPC << "\tSet word: 0x" << addr << " value: " << std::to_string(data) << std::endl;
#endif // DEBUG
}

void SCC68070::SetLong(const uint32_t& addr, const uint32_t& data)
{
    if(addr < 0x80000000 || addr >= 0xC0000000)
        vdsc->SetLong(addr, data);
#ifdef DEBUG
    else
        out << "OUT OF RANGE ";
    out << std::hex << currentPC << "\tSet Long: 0x" << addr << " value: " << std::to_string(data) << std::endl;
#endif // DEBUG
}
