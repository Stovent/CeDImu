#include "BIOS.hpp"

#include <cstring>

namespace OS9
{

/** \brief OS9 Module Additional Header.
 * \param memory A pointer to the beginning of the additional header.
 */
ModuleExtraHeader::ModuleExtraHeader(const uint8_t* memory) :
    M_Exec((uint32_t)memory[0x00] << 24 | memory[0x01] << 16 | memory[0x02] << 8 | memory[0x03]),
    M_Excpt((uint32_t)memory[0x04] << 24 | memory[0x05] << 16 | memory[0x06] << 8 | memory[0x07]),
    M_Mem((uint32_t)memory[0x08] << 24 | memory[0x09] << 16 | memory[0x0A] << 8 | memory[0x0B]),
    M_Stack((uint32_t)memory[0x0C] << 24 | memory[0x0D] << 16 | memory[0x0E] << 8 | memory[0x0F]),
    M_IData((uint32_t)memory[0x10] << 24 | memory[0x11] << 16 | memory[0x12] << 8 | memory[0x13]),
    M_IRefs((uint32_t)memory[0x14] << 24 | memory[0x15] << 16 | memory[0x16] << 8 | memory[0x17]),
    M_Init((uint32_t)memory[0x18] << 24 | memory[0x19] << 16 | memory[0x1A] << 8 | memory[0x1B]),
    M_Term((uint32_t)memory[0x1C] << 24 | memory[0x1D] << 16 | memory[0x1E] << 8 | memory[0x1F])
{}

/** \brief OS9 Module Header.
 * \param memory A pointer to the beginning of the header.
 * \param beg Its location in the BIOS memory area.
 */
ModuleHeader::ModuleHeader(const uint8_t* memory, const uint32_t beg) :
    M_SysRev((uint16_t)memory[0x02] << 8 | memory[0x03]),
    M_Size((uint32_t)memory[0x04] << 24 | memory[0x05] << 16 | memory[0x06] << 8 | memory[0x07]),
    M_Owner((uint32_t)memory[0x08] << 24 | memory[0x09] << 16 | memory[0x0A] << 8 | memory[0x0B]),
    M_Name((uint32_t)memory[0x0C] << 24 | memory[0x0D] << 16 | memory[0x0E] << 8 | memory[0x0F]),
    M_Accs((uint16_t)memory[0x10] << 8 | memory[0x11]),
    M_Type(memory[0x12]),
    M_Lang(memory[0x13]),
    M_Attr(memory[0x14]),
    M_Revs(memory[0x15]),
    M_Edit((uint16_t)memory[0x16] << 8 | memory[0x17]),
    extra(&memory[0x30]),
    name((char*)&memory[M_Name]),
    begin(beg),
    end(begin + M_Size)
{}

/** \brief OS9 BIOS.
 * \param bios The BIOS data.
 * \param sz The size of the BIOS data.
 * \param bs The base address of the BIOS in the memory map.
 */
BIOS::BIOS(const void* bios, const uint32_t sz, const uint32_t bs) :
    base(bs),
    size(sz),
    memory((uint8_t*)bios, (uint8_t*)bios + size)
{
    LoadModules();
}

BIOS::~BIOS()
{
}

void BIOS::LoadModules()
{
    for(uint32_t i = 0; i < size; i += 2)
    {
        if(memory[i] == 0x4A && memory[i + 1] == 0xFC)
        {
            uint16_t parity = 0xFFFF; // Header Parity Check
            for(int j = 0; j < 0x30; j += 2)
            {
                const uint16_t word = (uint16_t)memory[i + j] << 8 | memory[i + j + 1];
                parity ^= word;
            }
            if(parity == 0)
                modules.emplace_back(&memory[i], i);
        }
    }
}

/** \brief Get the module name the position is in.
 * \param offset The location in the BIOS area.
 * \return The name of the module if inside one, an empty string otherwise.
 */
std::string BIOS::GetModuleNameAt(const uint32_t offset) const
{
    for(const ModuleHeader& header : modules)
    {
        if(offset >= header.begin && offset < header.end)
            return header.name;
    }
    return "";
}

/** \brief Get the board type associated with the BIOS.
 * \return The board type, or Boards::Fail if the type could not be detected.
 */
Boards BIOS::GetBoardType() const
{
    const uint8_t id = memory[size - 4];
    switch(id >> 4 & 0xF)
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
bool BIOS::Has8KBNVRAM() const
{
    for(const OS9::ModuleHeader& mod : modules)
    {
        if(mod.name == "nvr")
        {
            uint16_t size = (uint16_t)memory[mod.begin + 74] << 8 | memory[mod.begin + 75];
            if(size == 0x1FF8) // 8KB
                return true;
        }
    }
    return false;
}

} // nampespace OS9
