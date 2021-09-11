#include "CeDImu.hpp"
#include "Config.hpp"
#include "GUI/MainFrame.hpp"
#include "CDI/CDI.hpp"

#include <wx/msgdlg.h>

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

constexpr float cpuSpeeds[17] = {
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

bool CeDImu::OnInit()
{
    Config::loadConfig();
    m_cpuSpeed = 8;

    new MainFrame(*this);

    InitCDI();
    if(m_cdi.board)
        m_cdi.board->cpu.Run();

    return true;
}

int CeDImu::OnExit()
{
    if(m_cdi.board)
        m_cdi.board->cpu.Stop(true);
    return 0;
}

bool CeDImu::InitCDI()
{
    std::ifstream biosFile(Config::systemBIOS, std::ios::binary | std::ios::in);
    if(!biosFile)
    {
        std::cerr << "Failed to open system BIOS '" << Config::systemBIOS << "'" << std::endl;
        wxMessageBox("Failed to open system BIOS '" + Config::systemBIOS + "'. Please check the BIOS path in the settings.");
        return false;
    }

    biosFile.seekg(0, std::ios::end);
    size_t biosSize = biosFile.tellg();
    biosFile.seekg(0);

    std::unique_ptr<uint8_t[]> bios = std::make_unique<uint8_t[]>(biosSize);
    biosFile.read((char*)bios.get(), biosSize);

    std::unique_ptr<uint8_t[]> nvram = nullptr;
    std::string filename = std::filesystem::path(Config::systemBIOS).filename().string();
    std::ifstream nvramFile("nvram_" + filename + ".bin");
    if(nvramFile)
    {
        nvramFile.seekg(0, std::ios::end);
        size_t nvramSize = nvramFile.tellg();
        nvramFile.seekg(0);
        nvram = std::make_unique<uint8_t[]>(nvramSize);
        nvramFile.read((char*)nvram.get(), nvramSize);
    }
    else
        std::cout << "Warning: no NVRAM file associated with the system BIOS used" << std::endl;

    std::lock_guard<std::mutex> lock(m_cdiMutex);
    m_cdi.config.has32KBNVRAM = Config::has32KBNVRAM;
    m_cdi.config.PAL = Config::PAL;
    if(Config::initialTime.size() == 0)
        m_cdi.config.initialTime = time(NULL);
    else
        m_cdi.config.initialTime = stoi(Config::initialTime);

    return m_cdi.LoadBoard(bios.get(), biosSize, nvram.get(), Config::boardType);
}
