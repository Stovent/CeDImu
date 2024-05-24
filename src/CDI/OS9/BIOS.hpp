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
    explicit ModuleExtraHeader(const uint8_t* memory) noexcept;

    uint32_t M_Exec;
    uint32_t M_Excpt;
    uint32_t M_Mem;
    uint32_t M_Stack;
    uint32_t M_IData;
    uint32_t M_IRefs;
    uint32_t M_Init;
    uint32_t M_Term;
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

    uint16_t   M_SysRev;
    uint32_t   M_Size;
    uint32_t   M_Owner;
    uint32_t   M_Name; // Module Name Offset
    uint16_t   M_Accs;
    ModuleType M_Type;
    uint8_t    M_Lang;
    uint8_t    M_Attr;
    uint8_t    M_Revs;
    uint16_t   M_Edit;
    ModuleExtraHeader extra;

    std::string name;
    uint32_t begin;
    uint32_t end;
};

class BIOS
{
public:
    explicit BIOS(std::span<const uint8_t> bios);

    uint32_t GetSize() const noexcept { return m_memory.size(); }

    const uint8_t& At(const uint32_t addr) const { return m_memory.at(addr); }

    /** \brief Returns the byte at \p offset.
     * \param offset The location of the byte in the BIOS area. */
    const uint8_t& operator[](const uint32_t offset) const noexcept { return m_memory[offset]; }

    std::string GetModuleNameAt(const uint32_t offset) const;

    Boards GetBoardType() const noexcept;

    bool Has8KBNVRAM() const noexcept;

    const std::vector<ModuleHeader>& GetModules() const noexcept { return m_modules; }

    bool ReplaceModule(std::span<const uint8_t> module);

private:
    std::vector<uint8_t> m_memory;
    std::vector<ModuleHeader> m_modules{}; /**< OS9 modules inside the BIOS. */

    void LoadModules();

    using ModuleIterator = decltype(m_modules)::iterator;
};

} // nampespace OS9

#endif // CDI_OS9_BIOS_HPP
