#ifndef CDI_BOARDS_SOFTCDISCHEDULER_SOFTCDISCHEDULER_HPP
#define CDI_BOARDS_SOFTCDISCHEDULER_SOFTCDISCHEDULER_HPP

#include "../../CDI.hpp"
#include "../../cores/CDDrive/CDDrive.hpp"

/** \brief The scheduler and system call dispatcher only, making SoftCDI reusable with any board type. */
class SoftCDIScheduler : virtual public CDI
{
public:
    SoftCDIScheduler(CDIDisc disc = CDIDisc());
    virtual ~SoftCDIScheduler();

    virtual CDIDisc& GetDisc() noexcept override;

protected:
    virtual void Scheduler(std::stop_token stopToken) override;
    /** \brief Increments the emulated time for the SoftCDI components and CDI's.
     * \warning This method calls `CDI::IncrementTime(ns);`.
     */
    virtual void IncrementTime(double ns) override;
    /** \brief Increments the emulated time for the SoftCDI components ONLY, not any of its parents of children.
     * \warning This method does NOT call CDI::IncrementTime().
     */
    void LocalIncrementTime(double ns);

    virtual void Reset(bool resetCPU) override;
    void LocalReset();

    CDDrive m_cdDrive;

    // System call handling.
    void DispatchSystemCall(uint16_t syscall) noexcept;

    void SoftCDIDebug() noexcept;
    void CDDrivePlay() noexcept;
    void CDDriveDmaSector() noexcept;
    void CDDriveGetSubheader() noexcept;

    /** \brief SoftCDI system calls.
     * TODO: organise this list.
     */
    enum SystemCalls : uint16_t
    {
        _Min = 0x100, /**< Minimal syscall index to not overlap with OS-9. */
        SoftCDI_Debug = 0x100, /**< Not stable system call that does nothing, used for debug purposes. */
        CdDrivePlay = 0x101,
        CdDriveDmaSector = 0x102,
        CdDriveGetSubheader = 0x103,
        UCMGetStat = 0x104,
        UCMSetStat = 0x105,
    };
};

#endif // CDI_BOARDS_SOFTCDISCHEDULER_SOFTCDISCHEDULER_HPP
