#ifndef OS9_BIOS_HPP
#define OS9_BIOS_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace OS9
{

struct ModuleExtraHeader
{
    explicit ModuleExtraHeader(const uint8_t* memory);

    const uint32_t M_Exec;
    const uint32_t M_Excpt;
    const uint32_t M_Mem;
    const uint32_t M_Stack;
    const uint32_t M_IData;
    const uint32_t M_IRefs;
    const uint32_t M_Init;
    const uint32_t M_Term;
};

struct ModuleHeader
{
    explicit ModuleHeader(const uint8_t* memory, const uint32_t beg);

    const uint16_t M_SysRev;
    const uint32_t M_Size;
    const uint32_t M_Owner;
    const uint32_t M_Name; // Module Name Offset
    const uint16_t M_Accs;
    const uint8_t  M_Type;
    const uint8_t  M_Lang;
    const uint8_t  M_Attr;
    const uint8_t  M_Revs;
    const uint16_t M_Edit;
    const ModuleExtraHeader extra;

    const std::string name;
    const uint32_t begin;
    const uint32_t end;
};

class BIOS
{
    const uint32_t size;
    uint8_t* memory; // TODO: allocate on stack?

    std::vector<ModuleHeader> modules;

    void LoadModules();

public:
    BIOS() = delete;
    BIOS(const void* bios, const uint32_t sz);
    ~BIOS();

    inline uint8_t operator[](const uint32_t offset) const { return memory[offset]; }
    inline const uint8_t* operator()() const { return memory; }

    std::string GetPositionInformation(const uint32_t offset) const;
};

} // nampespace OS9

#endif // OS9_BIOS_HPP
