#ifndef CEDIMU_HPP
#define CEDIMU_HPP

#include "Config.hpp"

#include "CDI/CDI.hpp"

#include <wx/app.h>

#include <fstream>
#include <mutex>

extern const double CPU_SPEEDS[];
extern const uint64_t SPEED_NUMERATORS[];

class CeDImu : public wxApp
{
public:
    std::recursive_mutex m_cdiMutex; // To lock before accessing m_cdi.
    std::unique_ptr<CDI> m_cdi;
    CDIDisc m_disc;
    uint16_t m_cpuSpeed;
    double m_emuSpeedDelay{0.0}; /**< Delta in nanoseconds between two frames for emulation speed. */
    Callbacks m_callbacks;

    std::string m_biosName;
    std::ofstream m_uartOut;
    std::ofstream m_instructionsOut;
    std::ofstream m_exceptionsOut;
    std::ofstream m_memoryAccessOut;

    virtual bool OnInit() override;
    virtual int OnExit() override;

    bool InitCDI(const Config::BiosConfig& biosConfig);
    bool OpenDisc(const std::string& filename);
    void CloseDisc();
    CDIDisc& GetDisc();

    void StartEmulation();
    void StopEmulation();
    /** \brief Returns the delay between two frames to match the emulation speed.
     * \returns The delay in nanoseconds.
     * Emulation speed is not clocking the emulated cores, it simply runs the system faster or slower than real time.
     */
    uint64_t GetEmulationSpeedFrameDelay() const noexcept { return SPEED_NUMERATORS[m_cpuSpeed] / m_cdi->GetFrameRate(); }
    void IncreaseEmulationSpeed();
    void DecreaseEmulationSpeed();

    void WriteInstruction(const LogInstruction& inst);
    void WriteException(const LogSCC68070Exception& e, size_t trapIndex);
    void WriteRTE(uint32_t pc, uint16_t format, const LogSCC68070Exception& e, size_t trapIndex);
    void WriteMemoryAccess(const LogMemoryAccess& log);

    void SetOnLogDisassembler(const std::function<void(const LogInstruction&)>& callback);
    void SetOnUARTOut(const std::function<void(uint8_t)>& callback);
    void SetOnFrameCompleted(const std::function<void(const Video::Plane&)>& callback);
    void SetOnSaveNVRAM(const std::function<void(const void*, size_t)>& callback);
    void SetOnLogICADCA(const std::function<void(Video::ControlArea, LogICADCA)>& callback);
    void SetOnLogMemoryAccess(const std::function<void(const LogMemoryAccess&)>& callback);
    void SetOnLogException(const std::function<void(const LogSCC68070Exception&)>& callback);
    void SetOnLogRTE(const std::function<void(uint32_t, uint16_t)>& callback);
};

#endif // CEDIMU_HPP
