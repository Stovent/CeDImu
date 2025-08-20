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

constexpr double CPU_SPEEDS[] = {
    0.015625,
    0.03125,
    0.0625,
    0.125,
    0.25,
    0.33,
    0.5,
    0.75,
    1.0,
    1.5,
    2,
    3,
    4,
    6,
    8,
    16,
    32,
    64,
};

constexpr uint64_t SPEED_NUMERATORS[] = {
    64'000'000'000,
    32'000'000'000,
    16'000'000'000,
     8'000'000'000,
     4'000'000'000,
     3'000'000'000,
     2'000'000'000,
     1'333'333'333,
     1'000'000'000,
       666'666'666,
       500'000'000,
       333'333'333,
       250'000'000,
       166'666'666,
       125'000'000,
        62'500'000,
        31'250'000,
        15'625'000,
};
static_assert(sizeof(CPU_SPEEDS) == sizeof(SPEED_NUMERATORS));

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
    if(m_cdi)
        m_cdi->Stop(true);
    m_cdi.reset();
    return 0;
}

bool CeDImu::InitCDI(const Config::BiosConfig& biosConfig)
{
    {
        std::lock_guard<std::recursive_mutex> lock(m_cdiMutex);
        m_cdi.reset();
    }

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
    biosFile.read(reinterpret_cast<char*>(bios.get()), biosSize);

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
        nvramFile.read(reinterpret_cast<char*>(nvramBuffer.get()), nvramSize);
        nvram = std::span<const uint8_t>(nvramBuffer.get(), nvramSize);
    }
    else
        std::cout << "Warning: no NVRAM file associated with the system BIOS used" << std::endl;

    CDIConfig config {
        .PAL = biosConfig.PAL,
        .initialTime = std::nullopt,
        .has32KBNVRAM = biosConfig.has32KbNvram,
    };
    if(biosConfig.initialTime.empty())
        config.initialTime = time(NULL);
    else
    {
        const time_t time = stoll(biosConfig.initialTime);
        if(time != 0)
            config.initialTime = time;
    }

    SetOnSaveNVRAM([&] (const void* data, size_t size) {
        std::ofstream out(biosConfig.nvramFileName, std::ios::out | std::ios::binary);
        out.write(static_cast<const char*>(data), size);
        out.close();
    });

    try
    {
        std::lock_guard<std::recursive_mutex> lock(m_cdiMutex);
        m_cdi = CDI::NewCDI(biosConfig.boardType, biosConfig.useSoftCDIModules, std::span(bios.get(), biosSize), nvram, config, m_callbacks, std::move(m_disc));
    }
    catch(const std::exception& e)
    {
        wxMessageBox(std::string("Failed to load core: ") + e.what());
    }

    if(!m_cdi)
        return false;

    return true;
}

bool CeDImu::OpenDisc(const std::string& filename)
{
    if(m_cdi)
        return m_cdi->GetDisc().Open(filename);

    return m_disc.Open(filename);
}

void CeDImu::CloseDisc()
{
    m_disc.Close();
    if(m_cdi)
        m_cdi->GetDisc().Close();
}

CDIDisc& CeDImu::GetDisc()
{
    if(m_cdi)
        return m_cdi->GetDisc();

    return m_disc;
}

void CeDImu::StartEmulation()
{
    std::lock_guard<std::recursive_mutex> lock(m_cdiMutex);
    if(m_cdi)
        m_cdi->Run(true);
}

void CeDImu::StopEmulation()
{
    std::lock_guard<std::recursive_mutex> lock(m_cdiMutex);
    if(m_cdi)
        m_cdi->Stop(true);
}

void CeDImu::IncreaseEmulationSpeed()
{
    if(m_cpuSpeed < MAX_CPU_SPEED)
        ++m_cpuSpeed;
}

void CeDImu::DecreaseEmulationSpeed()
{
    if(m_cpuSpeed > 0)
        --m_cpuSpeed;
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

void CeDImu::SetOnLogDisassembler(const std::function<void(const LogInstruction&)>& callback)
{
    m_callbacks.SetOnLogDisassembler(callback);
    std::lock_guard<std::recursive_mutex> lock(m_cdiMutex);
    if(m_cdi)
        m_cdi->m_callbacks.SetOnLogDisassembler(callback);
}

void CeDImu::SetOnUARTOut(const std::function<void(uint8_t)>& callback)
{
    m_callbacks.SetOnUARTOut(callback);
    std::lock_guard<std::recursive_mutex> lock(m_cdiMutex);
    if(m_cdi)
        m_cdi->m_callbacks.SetOnUARTOut(callback);
}

void CeDImu::SetOnFrameCompleted(const std::function<void(const Video::Plane&)>& callback)
{
    m_callbacks.SetOnFrameCompleted(callback);
    std::lock_guard<std::recursive_mutex> lock(m_cdiMutex);
    if(m_cdi)
        m_cdi->m_callbacks.SetOnFrameCompleted(callback);
}

void CeDImu::SetOnSaveNVRAM(const std::function<void(const void*, size_t)>& callback)
{
    m_callbacks.SetOnSaveNVRAM(callback);
    std::lock_guard<std::recursive_mutex> lock(m_cdiMutex);
    if(m_cdi)
        m_cdi->m_callbacks.SetOnSaveNVRAM(callback);
}

void CeDImu::SetOnLogICADCA(const std::function<void(Video::ControlArea, LogICADCA)>& callback)
{
    m_callbacks.SetOnLogICADCA(callback);
    std::lock_guard<std::recursive_mutex> lock(m_cdiMutex);
    if(m_cdi)
        m_cdi->m_callbacks.SetOnLogICADCA(callback);
}

void CeDImu::SetOnLogMemoryAccess(const std::function<void(const LogMemoryAccess&)>& callback)
{
    m_callbacks.SetOnLogMemoryAccess(callback);
    std::lock_guard<std::recursive_mutex> lock(m_cdiMutex);
    if(m_cdi)
        m_cdi->m_callbacks.SetOnLogMemoryAccess(callback);
}

void CeDImu::SetOnLogException(const std::function<void(const LogSCC68070Exception&)>& callback)
{
    m_callbacks.SetOnLogException(callback);
    std::lock_guard<std::recursive_mutex> lock(m_cdiMutex);
    if(m_cdi)
        m_cdi->m_callbacks.SetOnLogException(callback);
}

void CeDImu::SetOnLogRTE(const std::function<void(uint32_t, uint16_t)>& callback)
{
    m_callbacks.SetOnLogRTE(callback);
    std::lock_guard<std::recursive_mutex> lock(m_cdiMutex);
    if(m_cdi)
        m_cdi->m_callbacks.SetOnLogRTE(callback);
}
