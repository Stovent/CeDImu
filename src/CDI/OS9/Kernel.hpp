#ifndef CDI_OS9_KERNEL_HPP
#define CDI_OS9_KERNEL_HPP

#include <cstdint>
#include <functional>
#include <span>
#include <utility>
#include <vector>

#include "../common/utils.hpp"

// TODO: correct namespace? or namespace CeDImu?
// namespace CeDImu
namespace OS9
{

/** \brief Memory access functions wrapper. */
class MemoryAccess
{
public:
    std::function<uint8_t(uint32_t)> GetByte;
    std::function<uint16_t(uint32_t)> GetWord;
    constexpr uint32_t GetLong(uint32_t addr) const noexcept { return as<uint32_t>(GetWord(addr)) << 16 | GetWord(addr + 2); }
};

// /** \brief Wraps the emulated memory for easy manipulation. */
// class EmulatedMemory
// {
// public:
//     EmulatedMemory() = delete;
//     EmulatedMemory(const size_t size) : m_memory(size, 0) {}

//     constexpr const uint8_t* Data() const noexcept { return m_memory.data(); }

//     /** \brief Returns the byte at the given address in memory.
//      * \param address The address of the byte.
//      * \return The byte.
//      */
//     // constexpr uint8_t operator[](const size_t address) const noexcept { return m_memory[address]; }
//     constexpr const uint8_t& operator[](const size_t address) const noexcept { return m_memory[address]; }

//     constexpr uint8_t& operator[](const size_t address) noexcept { return m_memory[address]; }

//     /** \brief Returns the size in bytes of the memory.
//      * \return The size in bytes.
//      */
//     constexpr size_t GetSize() const noexcept { return m_memory.size(); }

//     /** \brief Returns the value at the given address.
//      * \tparam T The type of the value to return.
//      * \param address The address of the value.
//      * \return The value.
//      */
//     template<typename T>
//     T Get(const size_t address) const noexcept { return T{&m_memory[address], &m_memory[address + T::SIZEOF]}; }

//     /*template<typename T>
//     struct Pointer
//     {
//         constexpr Pointer(MemoryAccess memoryAccess, uint32_t addr) : m_memory{memoryAccess}, m_address{addr} {}

//         constexpr uint32_t Address() const noexcept { return m_address; }

//         // T* operator->() noexcept { return std::addressof(m_t); }
//         // const T* operator->() const noexcept { return std::addressof(m_t); }

//         constexpr T operator*() const noexcept { return this[0]; }

//         constexpr T operator[](size_t index) const noexcept { return T{m_memory, m_address + as<uint32_t>(index * T::SIZEOF)}; }

//     private:
//         MemoryAccess m_memory;
//         uint32_t m_address; // /< The pointing to address value.
//     };*/

//     template<uint32_t OFFSET, typename T>
//     struct Member
//     {
//         Member(EmulatedMemory& memory) : m_memory{memory} {}

//         T Value()
//         {
//             // TODO: read the type T.
//             // return m_memory.GetWord(m_address + OFFSET);
//         }

//         // void Value(T val)
//         // {
//         //     // TODO: set value.
//         // }

//     private:
//         EmulatedMemory& m_memory;
//         uint32_t m_address; /**< Address of the parent struct in memory. */
//     };

// private:
//     std::vector<uint8_t> m_memory;
// };

/** \brief A type that can be read from emulated memory. */
template<typename T, size_t SIZE_OF>
class Type
{
public:
    static constexpr size_t SIZEOF = SIZE_OF;
    virtual ~Type() noexcept {};
    /** \brief Reads the current value of the data from emulated memory. */
    virtual T Read() const noexcept = 0;
};

template<typename T>
struct Pointer // : public Type<Pointer, 4>
{
    template<typename... Args>
    Pointer(MemoryAccess& memory, uint32_t addr) : m_memory{memory}, m_address{addr}, m_t(m_memory, m_address) {}

    /** \brief Address of the pointer itself. */
    constexpr uint32_t PointerAddress() const noexcept { return m_address; }
    /** \brief Value of the pointer. */
    constexpr uint32_t TargetAddress() const noexcept { return m_target; }

    constexpr T* operator->() noexcept { Load(); return std::addressof(m_t); }
    constexpr const T* operator->() const noexcept { Load(); return std::addressof(m_t); }

    constexpr const T& operator*() const noexcept { Load(); return m_t; }

    constexpr T operator[](size_t index) const noexcept { return T{m_memory, m_target + as<uint32_t>(index * T::SIZEOF)}; }

private:
    /** \brief Reloads the stored value from memory. */
    constexpr void Load() const { m_target = m_memory.GetLong(m_address); m_t = T{m_memory, m_target}; }

    MemoryAccess m_memory;
    uint32_t m_address; /**< The pointing to address value. */
    mutable uint32_t m_target; /**< The target address. */
    mutable T m_t; // TODO: should it be mutable? or the method non-const? I guess should be non-const.
};

/** \brief Big-endian uint16_t. */
class U16 : public Type<uint16_t, 2>
{
public:
    U16() = delete;
    U16(MemoryAccess memoryAccess, uint32_t address) : m_memory{memoryAccess}, m_address{address} {}

    /** \brief Actually read the value from the emulated memory. */
    uint16_t Read() const noexcept override
    {
        return m_memory.GetWord(m_address);
    }

private:
    MemoryAccess m_memory;
    uint32_t m_address;
};

/** \brief Big-endian uint32_t. */
class U32 : public Type<uint32_t, 4>
{
public:
    U32() = delete;
    U32(MemoryAccess memoryAccess, uint32_t address) : m_memory{memoryAccess}, m_address{address} {}

    /** \brief Actually read the value from the emulated memory. */
    uint32_t Read() const noexcept override
    {
        return m_memory.GetLong(m_address);
    }

private:
    MemoryAccess m_memory;
    uint32_t m_address;
};

/** \brief Module directory entry. */
struct ModuleDirectoryEntry // : public EmulatedMemory::Type<uint16_t, 2>
{
    static constexpr size_t SIZEOF = 0x10;
    ModuleDirectoryEntry(MemoryAccess memory, uint32_t address)
        : MD_MPtr{memory, address}
        , MD_Group{memory, address}
        , MD_Static{memory, address}
        , MD_Link{memory, address}
        , MD_MChk{memory, address}
    {}

    // EmulatedMemory::Pointer<Module> MD_MPtr; /**< Address of the module. */
    Pointer<U16> MD_MPtr; /**< Address of the module. */
    U32 MD_Group; /**< Module group identifier. */
    U32 MD_Static; /**< Size of the memory area allocated to contain the module group. */
    U16 MD_Link; /**< Link count of the module. */
    U16 MD_MChk; /**< Check word calculated from the module header bytes. */
};

/** \brief OS-9 System Globals.
 * Refer to The OS-9 Guru for a description of it.
 * Not all of it is declared here, I only use it for the purpose of finding kernel data.
 */
struct SystemGlobals // : public EmulatedMemory::Type<SystemGlobals>
{
    SystemGlobals(MemoryAccess memory, uint32_t address)
        : D_ID{memory, address}
        , D_ModDir{memory, address + 0x3C}
        , D_ModDirEnd{memory, address + 0x40}
        // , D_PrcDBT{memory}
        // , D_PthDBT{memory}
        // , D_Proc{memory}
    {}

    U16 D_ID;
    Pointer<ModuleDirectoryEntry> D_ModDir;
    Pointer<ModuleDirectoryEntry> D_ModDirEnd;
    // EmulatedMemory::Member<0x40, void*> D_ModDirEnd;
    // EmulatedMemory::Member<0x44, void*> D_PrcDBT;
    // EmulatedMemory::Member<0x48, void*> D_PthDBT;
    // EmulatedMemory::Member<0x4C, void*> D_Proc; /**< The current process. */
};

// FUCKING obvious but use a pointer when the address is unknown and held somewhere.
// Use a struct directly when the variable's location is known and constant.

/** \brief Kernel state in emulated memory.
 * TODO: is it really necessary? You can directly have a pointer to the SystemGlobals.
 * I guess add useful features to this so it can work.
 */
class Kernel
{
public:
    Kernel() = delete;
    /** \brief Constructs a kernel state.
     * \param getWord Callback used to read memory. Must not modify the original data.
     */
    Kernel(MemoryAccess memoryAccess)
        : m_memory{memoryAccess}
        , m_systemGlobals{memoryAccess, 0} // TODO: it is known at runtime.
    {
        // TODO: if 0, do nothing.
    }

// private:
    MemoryAccess m_memory;

    Pointer<SystemGlobals> m_systemGlobals; /**< System Globals structure. */
    // SystemGlobals m_systemGlobals; /**< System Globals structure. */
};

} // namespace OS9

#endif // CDI_OS9_KERNEL_HPP
