#ifndef SCC66470_HPP
#define SCC66470_HPP

class SCC66470;

#include "../VDSC.hpp"

class SCC66470 : public VDSC
{
    uint8_t* memory;
    uint16_t* internalRegisters;
    uint8_t memorySwapCount;
    bool stopOnNextframe;
    const bool isMaster;
    std::function<void()> OnFrameCompleted;

    std::ofstream out_dram;

public:
    explicit SCC66470(Board* board, const bool ismaster);
    virtual ~SCC66470();

    virtual void Reset() override;

    virtual bool LoadBIOS(const void* bios, const uint32_t size) override;
    virtual void PutDataInMemory(const void* s, unsigned int size, unsigned int position) override;
    virtual void MemorySwap() override;

    virtual uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Log | Trigger) override;
    virtual uint16_t GetWord(const uint32_t addr, const uint8_t flags = Log | Trigger) override;
    virtual uint32_t GetLong(const uint32_t addr, const uint8_t flags = Log | Trigger) override;

    virtual void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Log | Trigger) override;
    virtual void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Log | Trigger) override;
    virtual void SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags = Log | Trigger) override;

    virtual void DrawLine() override;
    virtual inline uint32_t GetLineDisplayTimeNanoSeconds() override
    {
        return 41000;
    }

    virtual void SetOnFrameCompletedCallback(std::function<void()> callback) override;
    virtual void StopOnNextFrame(const bool stop = true) override;

    virtual std::vector<VDSCRegister> GetInternalRegisters() override;
    virtual std::vector<VDSCRegister> GetControlRegisters() override;
    virtual wxImage GetScreen() override;
    virtual wxImage GetPlaneA() override;
    virtual wxImage GetPlaneB() override;
    virtual wxImage GetBackground() override;
    virtual wxImage GetCursor() override;
};

#endif // SCC66470_HPP
