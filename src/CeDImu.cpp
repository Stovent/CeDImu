#include "CeDImu.hpp"
#include "Config.hpp"
#include "GUI/MainFrame.hpp"
#include "CDI/CDI.hpp"

#include <wx/msgdlg.h>

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>

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

    return true;
}

int CeDImu::OnExit()
{
    Config::saveConfig();
    return 0;
}

bool CeDImu::InitCDI()
{
    std::ifstream biosFile(Config::systemBIOS, std::ios::binary | std::ios::in);
    if(!biosFile)
    {
        std::cerr << "Failed to open system BIOS '" << Config::systemBIOS << "'" << std::endl;
        wxMessageBox("Failed to open system BIOS '" + Config::systemBIOS + "'");
        return false;
    }

    biosFile.seekg(0, std::ios::end);
    size_t biosSize = biosFile.tellg();
    biosFile.seekg(0);

    uint8_t* bios = new uint8_t[biosSize];
    biosFile.read((char*)bios, biosSize);

    uint8_t* nvram = nullptr;
    std::filesystem::path filename = std::filesystem::path(Config::systemBIOS).filename();
    std::ifstream nvramFile("nvram_" + filename.string() + ".bin");
    if(nvram)
    {
        nvramFile.seekg(0, std::ios::end);
        size_t nvramSize = nvramFile.tellg();
        nvramFile.seekg(0);
        nvram = new uint8_t[nvramSize];
        nvramFile.read((char*)nvram, nvramSize);
    }
    else
        std::cout << "Warning: no NVRAM file associated with the system BIOS found" << std::endl;

    m_cdi.config.has32KBNVRAM = Config::has32KBNVRAM;
    m_cdi.config.PAL = Config::PAL;
    if(Config::initialTime.size() == 0)
        m_cdi.config.initialTime = time(NULL);
    else
        m_cdi.config.initialTime = stoi(Config::initialTime);

    return m_cdi.LoadBoard(bios, biosSize, nvram, Config::boardType);
}
