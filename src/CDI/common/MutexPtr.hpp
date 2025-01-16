#ifndef CDI_COMMON_MUTEXPTR_HPP
#define CDI_COMMON_MUTEXPTR_HPP

#include <memory>
#include <mutex>
#include <utility>

/** \brief A Rust-like mutex that owns the object it protects using a std::unique_ptr instead of stored in-place. The object can only be accessed after locking it.
 * \tparam T The type encapsulated.
 * \tparam MUTEX The underlying mutex type.
 */
template<typename T, typename MUTEX = std::mutex>
class MutexPtr final
{
    // clang requires the members to be before the Guard class.
    MUTEX m_mutex{};
    std::unique_ptr<T> m_ptr;

public:
    /** \brief Object used to access the mutexed object and holds the mutex lock lifetime.
     * The Guard must never outlife its mutex.
     *
     * This object is also used to manipulate the underlying unique_ptr.
     */
    class Guard final
    {
    public:
        using UniquePtr = decltype(MutexPtr::m_ptr);
        using Pointer = UniquePtr::pointer;

        Guard() = delete;

        Guard(const Guard&) = delete;
        Guard& operator=(const Guard&) = delete;

        Guard(Guard&&) = delete;
        Guard& operator=(Guard&&) = delete;

        explicit Guard(MutexPtr& m)
            : m_mutex{m}
        {
            m_mutex.m_mutex.lock();
        }

        ~Guard()
        {
            m_mutex.m_mutex.unlock();
        }

        T& operator*() noexcept { return *m_mutex.m_ptr; }
        const T& operator*() const noexcept { return *m_mutex.m_ptr; }

        T* operator->() noexcept { return m_mutex.m_ptr.get(); }
        const T* operator->() const noexcept { return m_mutex.m_ptr.get(); }

        /** \brief Return whether the underlying UniquePtr contains a valid pointer. */
        explicit operator bool() const noexcept { return static_cast<bool>(m_mutex.m_ptr); }

        /** \brief Resets the underlying UniquePtr. */
        void Reset(Pointer p = Pointer()) noexcept { m_mutex.m_ptr.reset(p); }

        /** \brief Assigns a new UniquePtr to the managed object. */
        Guard& operator=(UniquePtr&& ptr) noexcept { m_mutex.m_ptr = std::move(ptr); return *this; }

        /** \brief Swaps the underlying UniquePtr with the given one. */
        void Swap(UniquePtr& other) noexcept { m_mutex.m_ptr.swap(other); }

    private:
        MutexPtr& m_mutex;
    };

    friend Guard;

    MutexPtr(const MutexPtr&) = delete;
    MutexPtr& operator=(const MutexPtr&) = delete;

    MutexPtr(MutexPtr&&) = delete;
    MutexPtr& operator=(MutexPtr&&) = delete;

    /** \brief Initializes the Mutex to hold no object in the underlying UniquePtr. */
    MutexPtr() : MutexPtr{nullptr} {}

    /** \brief Initializes the Mutex to hold no object in the underlying UniquePtr. */
    MutexPtr(std::nullptr_t) : m_ptr{nullptr} {}

    /** \brief Initializes the Mutex to hold an object constructed using the given arguments. */
    template<class... Args>
    explicit MutexPtr(Args&&... args)
        : m_ptr(std::make_unique<T, Args...>(std::forward<Args>(args)...))
    {}

    // No need for a destructor that unlocks the mutex, as its only supposed to be locked/unlocked by the Guard.

    /** \brief Locks the mutex and returns the guard to the object.
     *
     * To release the mutex, destroy the returned guard.
     */
    Guard Lock() noexcept
    {
        return Guard(*this);
    }
};

#endif // CDI_COMMON_MUTEXPTR_HPP
