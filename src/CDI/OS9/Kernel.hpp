/** \file Kernel.hpp
 * \brief Data structures to read data structures from emulated memory.
 * TODO: find a better name/namespace.
 */

#ifndef CDI_OS9_KERNEL_HPP
#define CDI_OS9_KERNEL_HPP

#include <cstdint>
#include <functional>
#include <span>
#include <utility>
#include <vector>

#include "../common/utils.hpp"

// namespace CeDImu
namespace OS9
{

/** \brief Memory access functions interface. */
class MemoryAccess
{
public:
    constexpr MemoryAccess() = default;
    constexpr MemoryAccess(const MemoryAccess&) = delete;
    constexpr MemoryAccess& operator=(const MemoryAccess&) = delete;
    constexpr MemoryAccess(MemoryAccess&&) = delete;
    constexpr MemoryAccess& operator=(MemoryAccess&&) = delete;
    constexpr virtual ~MemoryAccess() noexcept {}

    virtual constexpr uint8_t  GetByte(uint32_t addr) const noexcept = 0;
    virtual constexpr uint16_t GetWord(uint32_t addr) const noexcept = 0;
    virtual constexpr uint32_t GetLong(uint32_t addr) const noexcept { return as<uint32_t>(GetWord(addr)) << 16 | GetWord(addr + 2); }

    virtual constexpr void SetByte(uint32_t addr, uint8_t data) noexcept = 0;
    virtual constexpr void SetWord(uint32_t addr, uint16_t data) noexcept = 0;
    virtual constexpr void SetLong(uint32_t addr, uint32_t data) noexcept { SetWord(addr, data >> 16); SetWord(addr + 2, data); }
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
    // static constexpr size_t SIZEOF = SIZE_OF;

    constexpr Type() = default;
    constexpr Type(const Type&) = delete;
    constexpr Type& operator=(const Type&) = delete;
    constexpr Type(Type&&) = delete;
    constexpr Type& operator=(Type&&) = delete;
    constexpr virtual ~Type() noexcept {};

    /** \brief Reads the current value of the data from emulated memory. */
    virtual constexpr ValueType Read() const noexcept = 0;
    /** \brief Wrties the value of the data to emulated memory. */
    virtual constexpr void Write(const ValueType&) noexcept = 0;
};

template<typename T>
struct Pointer final
{
    using TargetType = T;

    constexpr Pointer(MemoryAccess& memory, uint32_t address)
        : m_memory{memory}, m_address{address} {}

    constexpr Pointer<TargetType> operator=(const uint32_t target_addr) noexcept
    {
        m_memory.SetLong(m_address, target_addr);
        return *this;
    }

    /** \brief Value of the pointer. */
    constexpr uint32_t TargetAddress() const noexcept { return m_memory.GetLong(m_address); }

    /** \brief Returns the TargetType struct initialized from the address in emulated memory. */
    constexpr TargetType operator*() const noexcept
    {
        const uint32_t addr = TargetAddress();
        return TargetType{m_memory, addr};
    }

private:

    MemoryAccess& m_memory;
    uint32_t m_address; /**< The address of the pointer itself. */
};

/** \brief uint8_t. */
class U8 final : public Type<uint8_t, 1>
{
public:
    constexpr U8() = delete;
    constexpr U8(MemoryAccess& memoryAccess, uint32_t address) : m_memory{memoryAccess}, m_address{address} {}

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

private:
    MemoryAccess& m_memory;
    uint32_t m_address;
};

/** \brief Big-endian uint16_t. */
class U16 final : public Type<uint16_t, 2>
{
public:
    constexpr U16() = delete;
    constexpr U16(MemoryAccess& memoryAccess, uint32_t address) : m_memory{memoryAccess}, m_address{address} {}

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

private:
    MemoryAccess& m_memory;
    uint32_t m_address;
};

/** \brief Big-endian uint32_t. */
class U32 final : public Type<uint32_t, 4>
{
public:
    constexpr U32() = delete;
    constexpr U32(MemoryAccess& memoryAccess, uint32_t address) : m_memory{memoryAccess}, m_address{address} {}

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

private:
    MemoryAccess& m_memory;
    uint32_t m_address;
};

/** \brief Null-terminated string. */
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

/** \brief Module. */
struct Module
{
    // static constexpr size_t SIZEOF = 0; // Array of modules doesn't exist.

    constexpr Module(MemoryAccess& memory, uint32_t address)
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
    {}

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
};

/** \brief Module directory entry. */
struct ModuleDirectoryEntry
{
    // static constexpr size_t SIZEOF = 0x10;

    constexpr ModuleDirectoryEntry(MemoryAccess& memory, uint32_t address)
        : MD_MPtr{memory, address}
        , MD_Group{memory, address + 0x4}
        , MD_Static{memory, address + 0x8}
        , MD_Link{memory, address + 0xC}
        , MD_MChk{memory, address + 0xE}
    {}

    Pointer<Module> MD_MPtr; /**< Address of the module. */
    U32 MD_Group; /**< Module group identifier. */
    U32 MD_Static; /**< Size of the memory area allocated to contain the module group. */
    U16 MD_Link; /**< Link count of the module. */
    U16 MD_MChk; /**< Check word calculated from the module header bytes. */
};

/** \brief OS-9 System Globals.
 * Refer to The OS-9 Guru for a description of it.
 * Not all of it is declared here, I only use it for the purpose of finding kernel data.
 */
struct SystemGlobals
{
    constexpr SystemGlobals(MemoryAccess& memory, uint32_t address)
        : D_ID{memory, address}
        , D_ModDir{memory, address + 0x3C}
        , D_ModDirEnd{memory, address + 0x40}
        // , D_PrcDBT{memory}
        // , D_PthDBT{memory}
        // , D_Proc{memory}
    {}

    U16 D_ID;
    Pointer<ModuleDirectoryEntry> D_ModDir; // Class that wraps the two with begin/end methods for iterator access?
    Pointer<ModuleDirectoryEntry> D_ModDirEnd;
    // EmulatedMemory::Member<0x44, void*> D_PrcDBT;
    // EmulatedMemory::Member<0x48, void*> D_PthDBT;
    // EmulatedMemory::Member<0x4C, void*> D_Proc; /**< The current process. */
};

/** \brief Kernel state in emulated memory.
 * TODO: is it really necessary? You can directly have a pointer to the SystemGlobals.
 * I guess add useful features to this so it can work.
 */
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
