#include "Kernel.hpp"

namespace OS9
{

bool Kernel::IsValid() const noexcept
{
    if(m_systemGlobals.TargetAddress() == 0)
        return false;

    SystemGlobals systemGlobals = *m_systemGlobals;
    return systemGlobals.D_ID.Read() == 0x4AFC;
}

std::string Kernel::GetModuleNameAt(const uint32_t addr) const
{
    if(!IsValid())
        return "";

    SystemGlobals systemGlobals = *m_systemGlobals;
    auto moduleDirectory = systemGlobals.D_ModDir;
    auto moduleDirectoryEnd = systemGlobals.D_ModDirEnd;

    const size_t len = moduleDirectoryEnd - moduleDirectory;
    for(size_t i = 0; i < len; ++i)
    {
        ModuleDirectoryEntry entry = moduleDirectory[i];
        const uint32_t modAddr = entry.MD_MPtr.TargetAddress();
        Module mod = *entry.MD_MPtr;

        uint32_t modSize = mod.M_Size.Read();
        if(addr >= modAddr && addr < (modAddr + modSize))
        {
            const uint32_t nameOffset = mod.M_Name.Read();
            return CString{m_memory, modAddr + nameOffset}.Read();
        }
    }

    return "";
}

} // namespace OS9
