#include "CeDImu.hpp"
#include "GUI/MainFrame.hpp"
#include "CDI/CDI.hpp"
#include "CDI/OS9/SystemCalls.hpp"

#include <wx/image.h>
#include <wx/msgdlg.h>

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <span>

constexpr float CPU_SPEEDS[] = {
    0.01,
    0.03,
    0.06,
    0.12,
    0.25,
    0.33,
    0.5,
    0.75,
    1.0,
    1.5,
    2,
    3,
    4,
    8,
    16,
    32,
    64,
};

static constexpr int DEFAULT_CPU_SPEED = 8;
static constexpr int MAX_CPU_SPEED = sizeof CPU_SPEEDS / sizeof *CPU_SPEEDS - 1;

bool CeDImu::OnInit()
{
    wxImage::AddHandler(new wxPNGHandler);
    Config::loadConfig();
    m_cpuSpeed = DEFAULT_CPU_SPEED;
    m_uartOut.open("uart_out", std::ios::out | std::ios::binary);

    new MainFrame(*this);

    if(Config::bioses.size() == 0)
        wxMessageBox("Welcome in CeDImu. Start by configuring your BIOSes in Options > Settings.\nSee the MANUAL.md file for more information.");

    return true;
}

int CeDImu::OnExit()
{
    m_cdi.UnloadBoard();
    return 0;
}

bool CeDImu::InitCDI(const Config::BiosConfig& biosConfig)
{
    std::ifstream biosFile(biosConfig.biosFilePath, std::ios::in | std::ios::binary);
    if(!biosFile)
    {
        std::cerr << "Failed to open system BIOS '" << biosConfig.biosFilePath << "'" << std::endl;
        wxMessageBox("Failed to open system BIOS '" + biosConfig.biosFilePath + "'. Please check the BIOS path in the settings.");
        return false;
    }

    biosFile.seekg(0, std::ios::end);
    size_t biosSize = biosFile.tellg();
    biosFile.seekg(0);

    std::unique_ptr<uint8_t[]> bios = std::make_unique<uint8_t[]>(biosSize);
    biosFile.read((char*)bios.get(), biosSize);

    std::unique_ptr<uint8_t[]> nvramBuffer = nullptr;
    m_biosName = biosConfig.name;
    std::ifstream nvramFile(biosConfig.nvramFileName, std::ios::in | std::ios::binary);

    std::span<const uint8_t> nvram;
    if(nvramFile)
    {
        nvramFile.seekg(0, std::ios::end);
        size_t nvramSize = nvramFile.tellg();
        nvramFile.seekg(0);
        nvramBuffer = std::make_unique<uint8_t[]>(nvramSize);
        nvramFile.read((char*)nvramBuffer.get(), nvramSize);
        nvram = std::span<const uint8_t>(nvramBuffer.get(), nvramSize);
    }
    else
        std::cout << "Warning: no NVRAM file associated with the system BIOS used" << std::endl;

    std::lock_guard<std::recursive_mutex> lock(m_cdiBoardMutex);
    m_cdi.config.has32KBNVRAM = biosConfig.has32KbNvram;
    m_cdi.config.PAL = biosConfig.PAL;
    if(biosConfig.initialTime.size() == 0)
        m_cdi.config.initialTime = time(NULL);
    else
        m_cdi.config.initialTime = stoi(biosConfig.initialTime);

    m_cdi.callbacks.SetOnSaveNVRAM([&] (const void* data, size_t size) {
        std::ofstream out(biosConfig.nvramFileName, std::ios::out | std::ios::binary);
        out.write((char*)data, size);
        out.close();
    });

    if(!m_cdi.LoadBoard(bios.get(), biosSize, nvram, biosConfig.boardType))
    {
        wxMessageBox("Failed to load system BIOS '" + biosConfig.biosFilePath + "': unsupported board type.");
        return false;
    }

    m_cdi.board->cpu.SetEmulationSpeed(CPU_SPEEDS[m_cpuSpeed]);
    return true;
}

void CeDImu::StartEmulation()
{
    std::lock_guard<std::recursive_mutex> lock(m_cdiBoardMutex);
    if(m_cdi.board)
        m_cdi.board->cpu.Run(true);
}

void CeDImu::StopEmulation()
{
    std::lock_guard<std::recursive_mutex> lock(m_cdiBoardMutex);
    if(m_cdi.board)
        m_cdi.board->cpu.Stop(true);
}

void CeDImu::IncreaseEmulationSpeed()
{
    if(m_cpuSpeed < MAX_CPU_SPEED)
    {
        std::lock_guard<std::recursive_mutex> lock(m_cdiBoardMutex);
        if(m_cdi.board)
            m_cdi.board->cpu.SetEmulationSpeed(CPU_SPEEDS[++m_cpuSpeed]);
    }
}

void CeDImu::DecreaseEmulationSpeed()
{
    if(m_cpuSpeed > 0)
    {
        std::lock_guard<std::recursive_mutex> lock(m_cdiBoardMutex);
        if(m_cdi.board)
            m_cdi.board->cpu.SetEmulationSpeed(CPU_SPEEDS[--m_cpuSpeed]);
    }
}

void CeDImu::WriteInstruction(const LogInstruction& inst)
{
    if(!m_instructionsOut.is_open())
        m_instructionsOut.open("instructions.txt");

    m_instructionsOut << std::setiosflags(std::ios::left)
                      << std::setw(8) << std::hex << inst.address
                      << std::setw(12) << inst.biosLocation
                      << inst.instruction
                      << std::endl;
}

void CeDImu::WriteException(const LogSCC68070Exception& e, size_t trapIndex)
{
    if(!m_exceptionsOut.is_open())
        m_exceptionsOut.open("exceptions.txt");
    if(!m_instructionsOut.is_open())
        m_instructionsOut.open("instructions.txt");


    m_exceptionsOut << std::setiosflags(std::ios::left)
                    << std::setw(8) << std::hex << e.returnAddress
                    << std::setw(12) << (e.vector == SCC68070::Trap0Instruction ? e.systemCall.module : "")
                    << std::setw(20) << e.disassembled;

    m_instructionsOut << std::setw(8) << std::hex << e.returnAddress
                      << std::setw(12) << (e.vector == SCC68070::Trap0Instruction ? e.systemCall.module : "")
                      << std::setw(20) << e.disassembled;

    if(e.vector == SCC68070::Trap0Instruction)
    {
        m_exceptionsOut << std::dec << '[' << trapIndex << "]  "
                        << std::setw(12) << OS9::systemCallNameToString(e.systemCall.type)
                        << e.systemCall.inputs;
        m_instructionsOut << std::dec << '[' << trapIndex << "]  "
                          << OS9::systemCallNameToString(e.systemCall.type);
    }

    m_exceptionsOut << std::endl;
    m_instructionsOut << std::endl;
}

void CeDImu::WriteRTE(uint32_t, uint16_t, const LogSCC68070Exception& e, size_t trapIndex)
{
    if(!m_exceptionsOut.is_open())
        m_exceptionsOut.open("exceptions.txt");

    m_exceptionsOut << std::setiosflags(std::ios::left)
                    << std::setw(8) << std::hex << e.returnAddress
                    << std::setw(12) << (e.vector == SCC68070::Trap0Instruction ? e.systemCall.module : "")
                    << "               RTE  "
                    << std::dec << '[' << trapIndex << "]  "
                    << std::setw(12) << OS9::systemCallNameToString(e.systemCall.type)
                    << e.systemCall.outputs << std::endl;
}

void CeDImu::WriteMemoryAccess(const LogMemoryAccess& log)
{
    if(!m_memoryAccessOut.is_open())
        m_memoryAccessOut.open("memory_access.txt");

    m_memoryAccessOut << std::setiosflags(std::ios::left)
                      << std::setw(14) << memoryAccessLocationToString(log.location)
                      << std::setw(8) << std::hex << log.pc
                      << std::setw(6) << log.direction
                      << std::setw(10) << log.type
                      << std::setw(10) << log.address
                      << std::setw(10) << std::dec << log.data << "  0x"
                      << std::hex << log.data
                      << std::endl;
}
