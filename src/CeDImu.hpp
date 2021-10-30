#ifndef CEDIMU_HPP
#define CEDIMU_HPP

#include "CDI/CDI.hpp"

#include <wx/app.h>

#include <fstream>
#include <mutex>

extern const float CPU_SPEEDS[];

class CeDImu : public wxApp
{
public:
    std::mutex m_cdiBoardMutex; // To lock only when accessing the board.
    CDI m_cdi;
    uint16_t m_cpuSpeed;

    std::string m_biosName;
    std::ofstream m_uartOut;
    std::ofstream m_instructionsOut;
    std::ofstream m_exceptionsOut;
    std::ofstream m_memoryAccessOut;

    virtual bool OnInit() override;
    virtual int OnExit() override;

    bool InitCDI();
    void StartEmulation();
    void StopEmulation();
    void IncreaseEmulationSpeed();
    void DecreaseEmulationSpeed();

    void WriteInstruction(const LogInstruction& inst);
    void WriteException(const LogSCC68070Exception& e, size_t trapIndex);
    void WriteRTE(uint32_t pc, uint16_t format, const LogSCC68070Exception& e, size_t trapIndex);
    void WriteMemoryAccess(const LogMemoryAccess& log);
};

#endif // CEDIMU_HPP
