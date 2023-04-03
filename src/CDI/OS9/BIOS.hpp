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
    ModuleHeader(const uint8_t* memory, const uint32_t beg);

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
public:
    const uint32_t m_base; /**< Base address of the BIOS in the memory map. */
    const uint32_t m_size; /**< Size of the BIOS in bytes. */

    std::vector<ModuleHeader> m_modules; /**< OS9 modules inside the BIOS. */

    BIOS(std::span<const uint8_t> bios, uint32_t base);

    BIOS(const BIOS&) = default;
    BIOS(BIOS&&) = default;

    /** \brief Returns the byte at \p offset.
        \param offset The location of the byte in the BIOS area. */
    inline uint8_t operator[](const uint32_t offset) const { return m_memory[offset]; }

    /** \brief Returns a pointer to the location \p pos.
        \param pos The location of the desired  pointer in the BIOS area. */
    inline const uint8_t* operator()(const uint32_t pos = 0) const { return &m_memory[pos]; }

    std::string GetModuleNameAt(const uint32_t offset) const;

    Boards GetBoardType() const;

    bool Has8KBNVRAM() const;

private:
    const std::vector<uint8_t> m_memory;

    void LoadModules();
};

} // nampespace OS9

#endif // CDI_OS9_BIOS_HPP
