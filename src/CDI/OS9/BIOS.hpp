#ifndef CDI_OS9_BIOS_HPP
#define CDI_OS9_BIOS_HPP

#include "../common/types.hpp"

#include <cstdint>
#include <span>
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
    enum ModuleType : uint8_t
    {
        NotUsed,
        Program,
        Subroutine,
        Multi,
        Data,
        CSDData,
        TrapLib = 11,
        System,
        FileManager,
        Driver,
        Descriptor,
    };

    ModuleHeader(const uint8_t* memory, const uint32_t beg);

    const uint16_t   M_SysRev;
    const uint32_t   M_Size;
    const uint32_t   M_Owner;
    const uint32_t   M_Name; // Module Name Offset
    const uint16_t   M_Accs;
    const ModuleType M_Type;
    const uint8_t    M_Lang;
    const uint8_t    M_Attr;
    const uint8_t    M_Revs;
    const uint16_t   M_Edit;
    const ModuleExtraHeader extra;

    const std::string name;
    const uint32_t begin;
    const uint32_t end;
};

class BIOS
{
public:
    explicit BIOS(std::span<const uint8_t> bios);

    BIOS(const BIOS&) = default;
    BIOS(BIOS&&) = default;

    uint32_t GetSize() const noexcept { return m_memory.size(); }

    const uint8_t& At(const uint32_t addr) const { return m_memory.at(addr); }

    /** \brief Returns the byte at \p offset.
        \param offset The location of the byte in the BIOS area. */
    const uint8_t& operator[](const uint32_t offset) const noexcept { return m_memory[offset]; }

    std::string GetModuleNameAt(const uint32_t offset) const;

    Boards GetBoardType() const noexcept;

    bool Has8KBNVRAM() const noexcept;

    const std::vector<ModuleHeader>& GetModules() const noexcept { return m_modules; }

private:
    const std::vector<uint8_t> m_memory;
    std::vector<ModuleHeader> m_modules{}; /**< OS9 modules inside the BIOS. */

    void LoadModules();
};

} // nampespace OS9

#endif // CDI_OS9_BIOS_HPP
