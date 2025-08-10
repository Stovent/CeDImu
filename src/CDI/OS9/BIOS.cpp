#include "BIOS.hpp"
#include "../common/utils.hpp"

#include <algorithm>
#include <cstring>

namespace OS9
{

/** \brief OS9 Module Additional Header.
 * \param memory A pointer to the beginning of the additional header.
 */
ModuleExtraHeader::ModuleExtraHeader(const uint8_t* memory) noexcept :
    M_Exec(getArray32(memory, 0x00)),
    M_Excpt(getArray32(memory, 0x04)),
    M_Mem(getArray32(memory, 0x08)),
    M_Stack(getArray32(memory, 0x0C)),
    M_IData(getArray32(memory, 0x10)),
    M_IRefs(getArray32(memory, 0x14)),
    M_Init(getArray32(memory, 0x18)),
    M_Term(getArray32(memory, 0x1C))
{}

/** \brief OS9 Module Header.
 * \param memory A pointer to the beginning of the header.
 * \param beg Its location in the BIOS memory area.
 */
ModuleHeader::ModuleHeader(const uint8_t* memory, const uint32_t beg) :
    M_SysRev(getArray16(memory, 0x02)),
    M_Size(getArray32(memory, 0x04)),
    M_Owner(getArray32(memory, 0x08)),
    M_Name(getArray32(memory, 0x0C)),
    M_Accs(getArray16(memory, 0x10)),
    M_Type(static_cast<ModuleType>(memory[0x12])),
    M_Lang(memory[0x13]),
    M_Attr(memory[0x14]),
    M_Revs(memory[0x15]),
    M_Edit(getArray16(memory, 0x16)),
    extra(&memory[0x30]),
    name(reinterpret_cast<const char*>(&memory[M_Name])),
    begin(beg),
    end(begin + M_Size)
{}

/** \brief OS9 BIOS.
 * \param bios The BIOS data.
 */
BIOS::BIOS(std::span<const uint8_t> bios)
    : m_memory(bios.begin(), bios.end())
{
    LoadModules();
}

/** \brief Get the module name the position is in.
 * \param offset The location in the BIOS area.
 * \return The name of the module if inside one, an empty string otherwise.
 */
std::string BIOS::GetModuleNameAt(const uint32_t offset) const
{
    for(const ModuleHeader& header : m_modules)
    {
        if(offset >= header.begin && offset < header.end)
            return header.name;
    }
    return "";
}

/** \brief Get the board type associated with the BIOS.
 * \return The board type, or Boards::Fail if the type could not be detected.
 */
Boards BIOS::GetBoardType() const noexcept
{
    const uint8_t id = bits<4, 7>(m_memory[GetSize() - 4]);
    switch(id)
    {
    case 2: return Boards::MiniMMC;
    case 3: return Boards::Mono1;
    case 4: return Boards::Mono2;
    case 5: return Boards::Roboco;
    case 6: return Boards::Mono3;
    case 7: return Boards::Mono4;
    default: return Boards::Fail;
    }
}

/** \brief Detect if the BIOS uses 8KB of NVRAM.
 * \return true if it does, false if not.
 */
bool BIOS::Has8KBNVRAM() const noexcept
{
    for(const OS9::ModuleHeader& mod : m_modules)
    {
        if(mod.name == "nvr")
        {
            const uint16_t size = getArray16(m_memory, mod.begin + 74);
            if(size == 0x1FF8) // 8KB
                return true;
        }
    }
    return false;
}

bool BIOS::ReplaceModule(std::span<const uint8_t> module)
{
    const char* name = reinterpret_cast<const char*>(&module[getArray32(module, 0x0C)]);
    const ModuleIterator old = std::find_if(m_modules.begin(), m_modules.end(), [&] (const ModuleHeader& header) -> bool { return header.name == name; });

    if(old != m_modules.end() && module.size() <= old->M_Size) // Overwrite the old module if the new one fits.
    {
        memset(&m_memory[old->begin], 0, old->M_Size);
        memcpy(&m_memory[old->begin], module.data(), module.size());

        *old = ModuleHeader(&m_memory[old->begin], old->begin);
        return true;
    }

    return false;
}

void BIOS::LoadModules()
{
    for(uint32_t i = 0; i < GetSize(); i += 2)
    {
        if(m_memory[i] == 0x4A && m_memory[i + 1] == 0xFC)
        {
            uint16_t parity = 0xFFFF; // Header Parity Check
            for(int j = 0; j < 0x30; j += 2)
            {
                const uint16_t word = getArray16(m_memory, i + j);
                parity ^= word;
            }
            if(parity == 0)
                m_modules.emplace_back(&m_memory[i], i);
        }
    }
}

} // nampespace OS9
