/** \file Kernel.hpp
 * \brief Data structures to read data structures from emulated memory.
 * TODO: find a better name/namespace.
 */

#ifndef CDI_OS9_KERNEL_HPP
#define CDI_OS9_KERNEL_HPP

#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "../common/Function.hpp"
#include "../common/utils.hpp"

// namespace CeDImu
namespace OS9
{

/** \brief Memory access functions. */
struct EmulatedMemoryAccess
{
    Function<uint8_t(uint32_t)> GetByte;
    Function<uint16_t(uint32_t)> GetWord;
    constexpr uint32_t GetLong(uint32_t addr) const noexcept { return as<uint32_t>(GetWord(addr)) << 16 | GetWord(addr + 2); }

    Function<void(uint32_t, uint8_t)> SetByte;
    Function<void(uint32_t, uint16_t)> SetWord;
    constexpr void SetLong(uint32_t addr, uint32_t data) const noexcept { SetWord(addr, data >> 16); SetWord(addr + 2, data); }
};

/** \brief Interface of a type that can be read from or written to emulated memory.
 * \tparam T The type.
 * \tparam SIZE_OF The size of the data in bytes in emulated memory.
 * TODO: is this interface necessary?
 */
template<typename T, size_t SIZE_OF>
class Type
{
public:
    using ValueType = T;
    static constexpr size_t SIZEOF = SIZE_OF;

    constexpr Type() = default;
    constexpr Type(const Type&) = delete;
    constexpr Type& operator=(const Type&) = delete;
    constexpr Type(Type&&) = delete;
    constexpr Type& operator=(Type&&) = delete;
    constexpr virtual ~Type() noexcept {};

    /** \brief Reads the current value of the data from emulated memory. */
    virtual constexpr ValueType Read() const noexcept = 0;
    /** \brief Writes the value of the data to emulated memory. */
    virtual constexpr void Write(const ValueType&) noexcept = 0;
    /** \brief Returns the address of the data. */
    virtual constexpr uint32_t Address() const noexcept = 0;
};

template<typename T>
struct Pointer final
{
    using TargetType = T;

    constexpr Pointer(const EmulatedMemoryAccess& memory, uint32_t address)
        : m_memory{memory}, m_address{address} {}

    /** \brief Returns the TargetType struct initialized from the address in emulated memory. */
    constexpr TargetType operator*() const noexcept
    {
        const uint32_t addr = TargetAddress();
        return TargetType{m_memory, addr};
    }

    constexpr Pointer<TargetType> operator=(const uint32_t target_addr) noexcept
    {
        m_memory.SetLong(m_address, target_addr);
        return *this;
    }

    constexpr TargetType operator[](size_t index) const noexcept
    {
        const uint32_t addr = TargetAddress() + as<uint32_t>(index * TargetType::SIZEOF);
        return TargetType{m_memory, addr};
    }

    /** \brief Value of the pointer. */
    constexpr uint32_t TargetAddress() const noexcept { return m_memory.GetLong(m_address); }
    /** \brief Returns the address of the pointer itself. */
    constexpr uint32_t PointerAddress() const noexcept { return m_address; }

    // TODO: how to implement operator++()? because it requires modifying the memory. But for local pointers we do not want this.
    // constexpr Pointer<TargetType>& operator++() noexcept { m_address += TargetType::SIZEOF; return *this; }

private:
    const EmulatedMemoryAccess& m_memory;
    uint32_t m_address; /**< The address of the pointer itself. */
};

/** \brief Compares the target address of both pointers. */
template<typename T>
constexpr bool operator==(const Pointer<T>& left, const Pointer<T>& right)
{
    return left.TargetAddress() == right.TargetAddress();
}

/** \brief Pointer subtraction which returns the difference in number of elements (and not in bytes). */
template<typename T>
constexpr ptrdiff_t operator-(const Pointer<T>& left, const Pointer<T>& right)
{
    return (left.TargetAddress() - right.TargetAddress()) / T::SIZEOF;
}

/** \brief A contiguous number of elements in emulated memory.
 * \tparam T The type of each element.
 * \tparam N The number of elements in the array.
 */
template<typename T/*, size_t N*/>
struct Array final
{
    using ElementType = T;

    constexpr Array(const EmulatedMemoryAccess& memory, uint32_t address)
        : m_memory{memory}, m_address{address} {}

    constexpr ElementType operator[](size_t index) const noexcept
    {
        const uint32_t addr = Address() + as<uint32_t>(index * ElementType::SIZEOF);
        return ElementType{m_memory, addr};
    }

    /** \brief Returns the address of the first element in the array. */
    constexpr uint32_t Address() const noexcept { return m_address; }

private:
    const EmulatedMemoryAccess& m_memory;
    uint32_t m_address; /**< The address of the first element in the array. */
};

/** \brief uint8_t. */
class U8 final : public Type<uint8_t, 1>
{
public:
    constexpr U8(const EmulatedMemoryAccess& memoryAccess, uint32_t address) : m_memory{memoryAccess}, m_address{address} {}

    /** \brief Actually read the value from the emulated memory. */
    constexpr ValueType Read() const noexcept override
    {
        return m_memory.GetByte(m_address);
    }

    constexpr operator ValueType() const noexcept
    {
        return Read();
    }

    constexpr void Write(const ValueType& d) noexcept override
    {
        m_memory.SetByte(m_address, d);
    }

    constexpr U8& operator=(const ValueType& d) noexcept
    {
        Write(d);
        return *this;
    }

    constexpr uint32_t Address() const noexcept override { return m_address; }

private:
    const EmulatedMemoryAccess& m_memory;
    uint32_t m_address;
};

/** \brief Big-endian uint16_t. */
class U16 final : public Type<uint16_t, 2>
{
public:
    constexpr U16(const EmulatedMemoryAccess& memoryAccess, uint32_t address) : m_memory{memoryAccess}, m_address{address} {}

    /** \brief Actually read the value from the emulated memory. */
    constexpr ValueType Read() const noexcept override
    {
        return m_memory.GetWord(m_address);
    }

    constexpr operator ValueType() const noexcept
    {
        return Read();
    }

    constexpr void Write(const ValueType& d) noexcept override
    {
        m_memory.SetWord(m_address, d);
    }

    constexpr U16& operator=(const ValueType& d) noexcept
    {
        Write(d);
        return *this;
    }

    constexpr uint32_t Address() const noexcept override { return m_address; }

private:
    const EmulatedMemoryAccess& m_memory;
    uint32_t m_address;
};

/** \brief Big-endian uint32_t. */
class U32 final : public Type<uint32_t, 4>
{
public:
    constexpr U32(const EmulatedMemoryAccess& memoryAccess, uint32_t address) : m_memory{memoryAccess}, m_address{address} {}

    /** \brief Actually read the value from the emulated memory. */
    constexpr ValueType Read() const noexcept override
    {
        return m_memory.GetLong(m_address);
    }

    constexpr operator ValueType() const noexcept
    {
        return Read();
    }

    constexpr void Write(const ValueType& d) noexcept override
    {
        m_memory.SetLong(m_address, d);
    }

    constexpr U32& operator=(const ValueType& d) noexcept
    {
        Write(d);
        return *this;
    }

    constexpr uint32_t Address() const noexcept override { return m_address; }

private:
    const EmulatedMemoryAccess& m_memory;
    uint32_t m_address;
};

/** \brief Null-terminated string. */
class CString
{
public:
    constexpr CString(const EmulatedMemoryAccess& memoryAccess, uint32_t address) : m_memory{memoryAccess}, m_address{address} {}

    /** \brief Actually read the value from the emulated memory. */
    constexpr std::string Read() const noexcept
    {
        std::string str;
        uint32_t addr = m_address;
        char c = m_memory.GetByte(addr);
        while(c != 0)
        {
            str += c;
            ++addr;
            c = m_memory.GetByte(addr);
        }
        return str;
    }

    constexpr operator std::string() const noexcept
    {
        return Read();
    }

    constexpr uint32_t Address() const noexcept { return m_address; }

private:
    const EmulatedMemoryAccess& m_memory;
    uint32_t m_address;
};

constexpr bool operator==(CString cstr, const char* constChar)
{
    return cstr.Read() == constChar;
}

constexpr bool operator==(const char* constChar, CString cstr)
{
    return cstr == constChar;
}

/** \brief Module. */
struct Module
{
    // static constexpr size_t SIZEOF = 0; // Array of modules doesn't exist.

    constexpr Module(const EmulatedMemoryAccess& memory, uint32_t address)
        : M_ID{memory, address}
        , M_SysRev{memory, address + 0x02}
        , M_Size{memory, address + 0x04}
        , M_Owner{memory, address + 0x08}
        , M_Name{memory, address + 0x0C}
        , M_Accs{memory, address + 0x10}
        , M_Type{memory, address + 0x12}
        , M_Lang{memory, address + 0x13}
        , M_Attr{memory, address + 0x14}
        , M_Revs{memory, address + 0x15}
        , M_Edit{memory, address + 0x16}
        , M_Usage{memory, address + 0x18}
        , M_Symbol{memory, address + 0x1C}
        , M_Parity{memory, address + 0x2E}
        , m_address{address}
    {}

    constexpr uint32_t Address() const noexcept { return m_address; }

    U16 M_ID; /**< Sync bytes. */
    U16 M_SysRev; /**< Revision ID. */
    U32 M_Size; /**< Module Size. */
    U32 M_Owner; /**< Owner ID. */
    U32 M_Name; /**< Module Name Offset. */
    U16 M_Accs; /**< Access Permissions. */
    U8 M_Type; /**< Module Type. */
    U8 M_Lang; /**< Module Lang. */
    U8 M_Attr; /**< Module Attributes. */
    U8 M_Revs; /**< Module Revision Level. */
    U16 M_Edit; /**< Edit Edition. */
    U32 M_Usage; /**< Usage Comment Offset. */
    U32 M_Symbol; /**< Symbol Table. */
    U16 M_Parity; /**< Module Header Parity Check. */

private:
    uint32_t m_address;
};

/** \brief Module directory entry. */
struct ModuleDirectoryEntry
{
    static constexpr size_t SIZEOF = 0x10;

    constexpr ModuleDirectoryEntry(const EmulatedMemoryAccess& memory, uint32_t address)
        : MD_MPtr{memory, address}
        , MD_Group{memory, address + 0x4}
        , MD_Static{memory, address + 0x8}
        , MD_Link{memory, address + 0xC}
        , MD_MChk{memory, address + 0xE}
        , m_address{address}
    {}

    constexpr uint32_t Address() const noexcept { return m_address; }

    Pointer<Module> MD_MPtr; /**< Address of the module. */
    U32 MD_Group; /**< Module group identifier. */
    U32 MD_Static; /**< Size of the memory area allocated to contain the module group. */
    U16 MD_Link; /**< Link count of the module. */
    U16 MD_MChk; /**< Check word calculated from the module header bytes. */

private:
    uint32_t m_address;
};

/** \brief Process Descriptor. */
struct ProcessDescriptor
{
    static constexpr size_t SIZEOF = 0x800;

    constexpr ProcessDescriptor(const EmulatedMemoryAccess& memory, uint32_t address)
        : P$ID{memory, address}
        , P$PID{memory, address + 0x2}
        , P$SID{memory, address + 0x4}
        , P$CID{memory, address + 0x6}
        , P$User{memory, address + 0x14}
        , P$Prior{memory, address + 0x18}
        , P$Age{memory, address + 0x1A}
        , P$State{memory, address + 0x1C}
        , P$QueuID{memory, address + 0x20}
        , P$QueueN{memory, address + 0x30}
        , P$QueueP{memory, address + 0x34}
        , P$PModul{memory, address + 0x38}
        , P$MemImg{memory, address + 0x1A8}
        , P$BlkSiz{memory, address + 0x228}
        , P$Data{memory, address + 0x32C}
        , P$DataSz{memory, address + 0x330}
        , m_address{address}
    {}

    constexpr uint32_t Address() const noexcept { return m_address; }

    U16 P$ID; /**< Process ID. */
    U16 P$PID; /**< Process ID of the parent. */
    U16 P$SID; /**< Process ID of the first sibling. */
    U16 P$CID; /**< Process ID of the first child. */
    U32 P$User; /**< Group number and user ID. */
    U16 P$Prior; /**< Process priority. */
    U16 P$Age; /**< Process age. */
    U16 P$State; /**< Process state (high byte only). */
    U8 P$QueuID; /**< ASCII charactere that indicates which queue the process is in. */
    Pointer<ProcessDescriptor> P$QueueN; /**< Next process in the queue. */
    Pointer<ProcessDescriptor> P$QueueP; /**< Previous process in the queue. */
    Pointer<Module> P$PModul; /**< Program module that was forked for this process. */
    Array<U32> P$MemImg; /**< Table of addresses of memory areas allocated. */
    Array<U32> P$BlkSiz; /**< Table of sizes of allocated memory areas. */
    U32 P$Data; /**< Address of the process' static storage. */
    U32 P$DataSz; /**< Size of the process' static storage. */

private:
    uint32_t m_address;
};

/** \brief OS-9 System Globals.
 * Refer to The OS-9 Guru for a description of it.
 * Not all of it is declared here, I only use it for the purpose of finding kernel data.
 */
struct SystemGlobals
{
    constexpr SystemGlobals(const EmulatedMemoryAccess& memory, uint32_t address)
        : D_ID{memory, address}
        , D_NoSleep{memory, address + 0x02}
        , D_Init{memory, address + 0x20}
        , D_Clock{memory, address + 0x24}
        , D_TckSec{memory, address + 0x28}
        , D_Year{memory, address + 0x2A}
        , D_Month{memory, address + 0x2C}
        , D_Day{memory, address + 0x2D}
        , D_Compat{memory, address + 0x2E}
        , D_68881{memory, address + 0x2F}
        , D_Julian{memory, address + 0x30}
        , D_Second{memory, address + 0x34}
        , D_ModDir{memory, address + 0x3C}
        , D_ModDirEnd{memory, address + 0x40}
        // , D_PrcDBT{memory}
        // , D_PthDBT{memory}
        , D_Proc{memory, address + 0x4C}
        , D_SysPrc{memory, address + 0x50}
        , D_Ticks{memory, address + 0x54}
        , D_TotRAM{memory, address + 0x6C}
        , D_MinBlk{memory, address + 0x70}
        , D_BlkSiz{memory, address + 0x7C}
        , m_address{address}
    {}

    constexpr uint32_t Address() const noexcept { return m_address; }

    U16 D_ID;
    U16 D_NoSleep;
    Pointer<Module> D_Init;
    U32 D_Clock;
    U16 D_TckSec;
    U16 D_Year;
    U8 D_Month;
    U8 D_Day;
    U8 D_Compat;
    U8 D_68881;
    U32 D_Julian;
    U32 D_Second;

    Pointer<ModuleDirectoryEntry> D_ModDir; // Class that wraps the two with begin/end methods for iterator access?
    Pointer<ModuleDirectoryEntry> D_ModDirEnd;
    // Pointer<ProcessDescriptor> D_PrcDBT;
    // Pointer<PathDescriptor> D_PthDBT;
    Pointer<ProcessDescriptor> D_Proc; /**< The current process. */
    Pointer<ProcessDescriptor> D_SysPrc; /**< System process descriptor. */
    U32 D_Ticks; /**< Tick count since coldstart. */

    U32 D_TotRAM;
    U32 D_MinBlk;
    U32 D_BlkSiz;

private:
    uint32_t m_address;
};

/** \brief Kernel state in emulated memory.
 * This class is a convenient API to interact with the kernel in memory.
 */
class Kernel
{
public:
    /** \brief Constructs a kernel state.
     * \param getWord Callback used to read memory. Must not modify the original data.
     */
    Kernel(EmulatedMemoryAccess memoryAccess)
        : m_memory{std::move(memoryAccess)}
        , m_systemGlobals{m_memory, 0} // The pointer itself is the initial stack pointer
    {
    }

    /** \brief Returns whether the system globals struct is valid and can be manipulated. */
    bool IsValid() const noexcept;

    /** \brief Searches the given address in the module list.
     * \param addr The address to search.
     * \return an empty string if not found.
     */
    std::string GetModuleNameAt(uint32_t addr) const;

    EmulatedMemoryAccess m_memory;
    Pointer<SystemGlobals> m_systemGlobals; /**< System Globals structure. */
};

} // namespace OS9

// namespace OS9
// {
//
// /** \brief Memory access functions wrapper. */
// struct MemoryAccess
// {
//     std::function<uint8_t(uint32_t)> GetByte;
//     std::function<uint16_t(uint32_t)> GetWord;
//     std::function<const uint8_t*(uint32_t)> GetPointer;
//     uint32_t GetLong(uint32_t addr) const noexcept { return as<uint32_t>(GetWord(addr)) << 16 | GetWord(addr + 2); }
// };
//
// /** \brief A type that can be read from emulated memory.
//  * \tparam SIZE_OF The size of the data in bytes in emulated memory.
//  */
// template<typename T, size_t SIZE_OF>
// class Type
// {
// public:
//     static constexpr size_t SIZEOF = SIZE_OF;
//     virtual ~Type() noexcept {};
//     /** \brief Reads the current value of the data from emulated memory. */
//     virtual T Read() const noexcept = 0;
// };
//
// template<typename T>
// struct Pointer // : public Type<Pointer, 4>
// {
//     constexpr Pointer(MemoryAccess& memory, uint32_t addr)
//         : m_memory{memory}
//         , m_address{addr}
//         , m_target{}
//         , m_t(m_memory, m_target)
//     {
//         LoadTarget();
//     }
//
//     /** \brief Address of the pointer itself. */
//     constexpr uint32_t PointerAddress() const noexcept { return m_address; }
//     /** \brief Value of the pointer. */
//     constexpr uint32_t TargetAddress() noexcept { return m_target; }
//
//     constexpr T* operator->() noexcept { LoadValue(); return std::addressof(m_t); }
//     constexpr const T* operator->() const noexcept { LoadValue(); return std::addressof(m_t); }
//
//     constexpr const T& operator*() noexcept { LoadValue(); return m_t; }
//
//     constexpr T operator[](size_t index) const noexcept { return T{m_memory, m_target + as<uint32_t>(index * T::SIZEOF)}; }
//
// private:
//     /** \brief Reloads the target address from memory. */
//     constexpr void LoadTarget() { m_target = m_memory.GetLong(m_address); }
//     /** \brief Reloads the stored value from memory. */
//     constexpr void LoadValue() { LoadTarget(); m_t = T{m_memory, m_target}; }
//
//     MemoryAccess m_memory;
//     uint32_t m_address; /**< The pointing to address value. */
//     uint32_t m_target; /**< The target address. */
//     T m_t;
// };
//
// /** \brief Pointer subtraction which returns the difference in number of elements (and not in bytes). */
// template<typename T>
// ssize_t operator-(Pointer<T>& left, Pointer<T>& right)
// {
//     return (left.TargetAddress() - right.TargetAddress()) / T::SIZEOF;
// }
//
// /** \brief uint8_t. */
// class U8 : public Type<uint8_t, 1>
// {
// public:
//     U8() = delete;
//     U8(MemoryAccess memoryAccess, uint32_t address) : m_memory{memoryAccess}, m_address{address} {}
//
//     /** \brief Actually read the value from the emulated memory. */
//     uint8_t Read() const noexcept override
//     {
//         return m_memory.GetByte(m_address);
//     }
//
// private:
//     MemoryAccess m_memory;
//     uint32_t m_address;
// };
//
// /** \brief Big-endian uint16_t. */
// class U16 : public Type<uint16_t, 2>
// {
// public:
//     U16() = delete;
//     U16(MemoryAccess memoryAccess, uint32_t address) : m_memory{memoryAccess}, m_address{address} {}
//
//     /** \brief Actually read the value from the emulated memory. */
//     uint16_t Read() const noexcept override
//     {
//         return m_memory.GetWord(m_address);
//     }
//
// private:
//     MemoryAccess m_memory;
//     uint32_t m_address;
// };
//
// /** \brief Big-endian uint32_t. */
// class U32 : public Type<uint32_t, 4>
// {
// public:
//     U32() = delete;
//     U32(MemoryAccess memoryAccess, uint32_t address) : m_memory{memoryAccess}, m_address{address} {}
//
//     /** \brief Actually read the value from the emulated memory. */
//     uint32_t Read() const noexcept override
//     {
//         return m_memory.GetLong(m_address);
//     }
//
// private:
//     MemoryAccess m_memory;
//     uint32_t m_address;
// };
//
// /** \brief Null-terminated string. */
// class CString : public Type<const char*, 4>
// {
// public:
//     CString() = delete;
//     CString(MemoryAccess memoryAccess, uint32_t address) : m_memory{memoryAccess}, m_address{address} {}
//
//     /** \brief Actually read the value from the emulated memory. */
//     const char* Read() const noexcept override
//     {
//         return reinterpret_cast<const char*>(m_memory.GetPointer(m_address));
//     }
//
// private:
//     MemoryAccess m_memory;
//     uint32_t m_address;
// };
//
// /** \brief Module. */
// struct Module // : public EmulatedMemory::Type<uint16_t, 2>
// {
//     static constexpr size_t SIZEOF = 0; // Array of modules doesn't exist.
//     Module(MemoryAccess memory, uint32_t address)
//         : M_ID{memory, address}
//         , M_SysRev{memory, address + 0x02}
//         , M_Size{memory, address + 0x04}
//         , M_Owner{memory, address + 0x08}
//         , M_Name{memory, address + 0x0C}
//         , M_Accs{memory, address + 0x10}
//         , M_Type{memory, address + 0x12}
//         , M_Lang{memory, address + 0x13}
//         , M_Attr{memory, address + 0x14}
//         , M_Revs{memory, address + 0x15}
//         , M_Edit{memory, address + 0x16}
//         , M_Usage{memory, address + 0x18}
//         , M_Symbol{memory, address + 0x1C}
//         , M_Parity{memory, address + 0x2E}
//     {}
//
//     // EmulatedMemory::Pointer<Module> MD_MPtr; /**< Address of the module. */
//     U16 M_ID; /**< Sync bytes. */
//     U16 M_SysRev; /**< Revision ID. */
//     U32 M_Size; /**< Module Size. */
//     U32 M_Owner; /**< Owner ID. */
//     U32 M_Name; /**< Module Name Offset. */
//     U16 M_Accs; /**< Access Permissions. */
//     U8 M_Type; /**< Module Type. */
//     U8 M_Lang; /**< Module Lang. */
//     U8 M_Attr; /**< Module Attributes. */
//     U8 M_Revs; /**< Module Revision Level. */
//     U16 M_Edit; /**< Edit Edition. */
//     U32 M_Usage; /**< Usage Comment Offset. */
//     U32 M_Symbol; /**< Symbol Table. */
//     U16 M_Parity; /**< Module Header Parity Check. */
// };
//
// /** \brief Module directory entry. */
// struct ModuleDirectoryEntry // : public EmulatedMemory::Type<uint16_t, 2>
// {
//     static constexpr size_t SIZEOF = 0x10;
//     ModuleDirectoryEntry(MemoryAccess memory, uint32_t address)
//         : MD_MPtr{memory, address}
//         , MD_Group{memory, address + 0x4}
//         , MD_Static{memory, address + 0x8}
//         , MD_Link{memory, address + 0xC}
//         , MD_MChk{memory, address + 0xE}
//     {}
//
//     Pointer<Module> MD_MPtr; /**< Address of the module. */
//     U32 MD_Group; /**< Module group identifier. */
//     U32 MD_Static; /**< Size of the memory area allocated to contain the module group. */
//     U16 MD_Link; /**< Link count of the module. */
//     U16 MD_MChk; /**< Check word calculated from the module header bytes. */
// };
//
// /** \brief OS-9 System Globals.
//  * Refer to The OS-9 Guru for a description of it.
//  * Not all of it is declared here, I only use it for the purpose of finding kernel data.
//  */
// struct SystemGlobals // : public EmulatedMemory::Type<SystemGlobals>
// {
//     SystemGlobals(MemoryAccess memory, uint32_t address)
//         : D_ID{memory, address}
//         , D_ModDir{memory, address + 0x3C}
//         , D_ModDirEnd{memory, address + 0x40}
//         // , D_PrcDBT{memory}
//         // , D_PthDBT{memory}
//         // , D_Proc{memory}
//     {}
//
//     U16 D_ID;
//     Pointer<ModuleDirectoryEntry> D_ModDir; // Class that wraps the two with begin/end methods for iterator access?
//     Pointer<ModuleDirectoryEntry> D_ModDirEnd;
//     // EmulatedMemory::Member<0x44, void*> D_PrcDBT;
//     // EmulatedMemory::Member<0x48, void*> D_PthDBT;
//     // EmulatedMemory::Member<0x4C, void*> D_Proc; /**< The current process. */
// };
//
// /** \brief Kernel state in emulated memory.
//  * TODO: is it really necessary? You can directly have a pointer to the SystemGlobals.
//  * I guess add useful features to this so it can work.
//  */
// class Kernel
// {
// public:
//     Kernel() = delete;
//     /** \brief Constructs a kernel state.
//      * \param getWord Callback used to read memory. Must not modify the original data.
//      */
//     Kernel(MemoryAccess memoryAccess)
//         : m_memory{memoryAccess}
//         , m_systemGlobals{memoryAccess, 0} // The pointer itself is the initial stack pointer
//     {
//     }
//
//     MemoryAccess m_memory;
//
//     Pointer<SystemGlobals> m_systemGlobals; /**< System Globals structure. */
// };
//
// } // namespace OS9

#endif // CDI_OS9_KERNEL_HPP
