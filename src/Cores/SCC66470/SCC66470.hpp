#ifndef SCC66470_HPP
#define SCC66470_HPP

class SCC66470;

#include <cstdint>
#include <string>

#include "../VDSC.hpp"

class SCC66470 : public VDSC
{
    uint8_t memorySwapCount;

public:
    SCC66470(CeDImu* appp);
    virtual ~SCC66470();

    virtual void Reset() override;

    virtual bool LoadBIOS(const char* filename) override;
    virtual void PutDataInMemory(const void* s, unsigned int size, unsigned int position) override;
    virtual void MemorySwap() override;

    virtual uint8_t  GetByteNoDebug(const uint32_t addr) override;
    virtual uint16_t GetWordNoDebug(const uint32_t addr) override;
    virtual uint32_t GetLongNoDebug(const uint32_t addr) override;

    virtual uint8_t  GetByte(const uint32_t addr) override;
    virtual uint16_t GetWord(const uint32_t addr) override;
    virtual uint32_t GetLong(const uint32_t addr) override;

    virtual void SetByte(const uint32_t addr, const uint8_t data) override;
    virtual void SetWord(const uint32_t addr, const uint16_t data) override;
    virtual void SetLong(const uint32_t addr, const uint32_t data) override;

    virtual void DisplayLine() override;
    virtual inline uint32_t GetLineDisplayTimeNanoSeconds() override
    {
        return 700;
    }

    virtual void OnFrameCompleted() override;
    virtual void ShowViewer() override {}
};

#endif // SCC66470_HPP
