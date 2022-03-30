#ifndef CDI_CORES_VDSC_HPP
#define CDI_CORES_VDSC_HPP

class CDI;
#include "../common/types.hpp"
#include "../OS9/BIOS.hpp"

#include <vector>


enum class ControlArea
{
    ICA1,
    DCA1,
    ICA2,
    DCA2,
};

/** \struct Plane
 * \brief Represent a video plane.
 */
struct Plane : public std::vector<uint8_t>
{
    static constexpr size_t MAX_WIDTH     = 768;
    static constexpr size_t MAX_HEIGHT    = 560;
    static constexpr size_t CURSOR_WIDTH  = 16;
    static constexpr size_t CURSOR_HEIGHT = 16;

    static constexpr size_t RGB_SIZE   = MAX_WIDTH * MAX_HEIGHT * 3;
    static constexpr size_t ARGB_SIZE  = MAX_WIDTH * MAX_HEIGHT * 4;
    static constexpr size_t CURSOR_ARGB_SIZE = CURSOR_WIDTH * CURSOR_HEIGHT * 4;

    uint16_t width; /**< Width of the plane. */
    uint16_t height; /**< Height of the plane. */

    explicit Plane(const size_t sz = ARGB_SIZE, uint16_t w = 0, uint16_t h = 0) : std::vector<uint8_t>(sz, 0), width(w), height(h) {}
    const uint8_t* operator()(size_t line, size_t pixelSize) const { return data() + line * width * pixelSize; }
    uint8_t* operator()(size_t line, size_t pixelSize) { return data() + line * width * pixelSize; }
};

struct RAMBank
{
    const uint8_t* data;
    uint32_t base;
    uint32_t size;
};

class VDSC
{
public:
    CDI& cdi;
    OS9::BIOS BIOS;
    uint32_t totalFrameCount;

    VDSC() = delete;
    VDSC(CDI& idc, const void* bios, const uint32_t size, const uint32_t base) : cdi(idc), BIOS(bios, size, base), totalFrameCount(0) {}
    virtual ~VDSC() {}

    virtual void Reset() = 0;
    virtual void IncrementTime(const double ns) = 0;

    virtual uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Log | Trigger) = 0;
    virtual uint16_t GetWord(const uint32_t addr, const uint8_t flags = Log | Trigger) = 0;

    virtual void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Log | Trigger) = 0;
    virtual void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Log | Trigger) = 0;

    virtual RAMBank GetRAMBank1() const = 0;
    virtual RAMBank GetRAMBank2() const = 0;

    virtual std::vector<InternalRegister> GetInternalRegisters() const = 0;
    virtual std::vector<InternalRegister> GetControlRegisters() const = 0;
    virtual const Plane& GetScreen() const = 0;
    virtual const Plane& GetPlaneA() const = 0;
    virtual const Plane& GetPlaneB() const = 0;
    virtual const Plane& GetBackground() const = 0;
    virtual const Plane& GetCursor() const = 0;
};

#endif // CDI_CORES_VDSC_HPP
