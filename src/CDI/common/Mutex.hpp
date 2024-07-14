#ifndef CDI_COMMON_MUTEX_HPP
#define CDI_COMMON_MUTEX_HPP

#include <memory>
#include <mutex>
#include <utility>

/** \brief A Rust-like mutex that owns the object it protects. The object can only be accessed after locking it.
 * \tparam T The type encapsulated.
 * \tparam MUTEX The underlying mutex type.
 */
template<typename T, typename MUTEX = std::mutex>
class Mutex final
{
public:
    /** \brief Object used to access the mutexed object and holds the the mutex lock lifetime.
     * The Guard must never outlife its mutex.
     */
    class Guard final
    {
    public:
        Guard() = delete;

        Guard(const Guard&) = delete;
        Guard& operator=(const Guard&) = delete;

        Guard(Guard&&) = delete;
        Guard& operator=(Guard&&) = delete;

        explicit Guard(Mutex& m)
            : m_mutex{m}
        {
            m_mutex.m_mutex.lock();
        }

        ~Guard()
        {
            m_mutex.m_mutex.unlock();
        }

        T& operator*() noexcept { return m_mutex.m_object; }
        const T& operator*() const noexcept { return m_mutex.m_object; }

        T* operator->() noexcept { return std::addressof(m_mutex.m_object); }
        const T* operator->() const noexcept { return std::addressof(m_mutex.m_object); }

    private:
        Mutex& m_mutex;
    };

    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    Mutex(Mutex&&) = delete;
    Mutex& operator=(Mutex&&) = delete;

    template<class... Args>
    explicit Mutex(Args&&... args)
        : m_object(std::forward<Args>(args)...)
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

private:
    friend Guard;
    MUTEX m_mutex{};
    T m_object;
};

#endif // CDI_COMMON_MUTEX_HPP
