#ifndef SCC66470_HPP
#define SCC66470_HPP

class SCC66470;

#include <cstdint>
#include <string>

#include "../VDSC.hpp"

class SCC66470 : public VDSC
{
public:
    SCC66470();
    ~SCC66470();

    void LoadBIOS(std::string filename) override;

    virtual int8_t  GetByte(const uint32_t& addr) const override;
    virtual int16_t GetWord(const uint32_t& addr) const override;
    virtual int32_t GetLong(const uint32_t& addr) const override;
    virtual void SetByte(const uint32_t& addr, const int8_t& data) override;
    virtual void SetWord(const uint32_t& addr, const int16_t& data) override;
    virtual void SetLong(const uint32_t& addr, const int32_t& data) override;

    void DisplayLine() override;
};

#endif // SCC66470_HPP
