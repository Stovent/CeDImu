#include "CeDImu.hpp"
#include "Config.hpp"

#include <wx/msgdlg.h>

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
    cpuSpeed = 8;
    stopOnNextFrame.store(false);

    uartOut.open("uart_out", std::ios::out | std::ios::binary);
    logInstructions.open("instructions.txt");
    logMemoryAccess = fopen("memory_access.txt", "w");

    mainFrame = new MainFrame(*this, "CeDImu", wxPoint(50, 50), wxSize(420, 310));
    mainFrame->Show(true);

    InitializeCores();
    return true;
}

int CeDImu::OnExit()
{
    StopGameThread();
    Config::saveConfig();
    uartOut.close();
    logInstructions.close();
    fclose(logMemoryAccess);
    return 0;
}

bool CeDImu::InitializeCores()
{
    FILE* f = fopen(Config::systemBIOS.c_str(), "rb");
    if(!f)
    {
        wxMessageBox("Could not open system BIOS file!\nPlease check the file in menu Config -> Settings");
        return false;
    }

    fseek(f, 0, SEEK_END);
    long biosSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t* bios = new uint8_t[biosSize];
    biosSize = fread(bios, 1, biosSize, f);
    fclose(f);

    CDIConfig config = {
        .PAL = Config::PAL,
        .initialTime = Config::initialTime.size() ? stoi(Config::initialTime) : time(nullptr),
    };
    cdi.config = config;
    cdi.LoadBoard(bios, biosSize, Boards::AutoDetect);
    delete[] bios;

#ifdef _WIN32
    biosName = Config::systemBIOS.substr(Config::systemBIOS.rfind('\\')+1);
#else
    biosName = Config::systemBIOS.substr(Config::systemBIOS.rfind('/')+1);
#endif // _WIN32

    cdi.callbacks.SetOnFrameCompleted([=] () -> void {
        if(this->stopOnNextFrame.load())
        {
            this->stopOnNextFrame.store(false);
            cdi.board->cpu.Stop(false);
            mainFrame->pauseItem->Check();
        }

        mainFrame->gamePanel->RefreshScreen();

        if(mainFrame->cpuViewer)
            mainFrame->cpuViewer->flushInstructions = true;

        if(mainFrame->vdscViewer)
        {
            mainFrame->vdscViewer->flushICA1 = true;
            mainFrame->vdscViewer->flushDCA1 = true;
            mainFrame->vdscViewer->flushICA2 = true;
            mainFrame->vdscViewer->flushDCA2 = true;
        }
    });

    cdi.callbacks.SetOnLogMemoryAccess([=] (const LogMemoryAccess& arg) {
        fprintf(logMemoryAccess, "[%5s] (0x%06X) %s %s at 0x%X : %d\n", arg.location.c_str(), arg.pc, arg.direction.c_str(), arg.size.c_str(), arg.address, arg.data);
    });

    cdi.board->cpu.SetEmulationSpeed(cpuSpeeds[cpuSpeed]);
    return true;
}

void CeDImu::IncreaseEmulationSpeed()
{
    if(cpuSpeed < 16)
    {
        cpuSpeed++;
        if(cdi.board)
            cdi.board->cpu.SetEmulationSpeed(cpuSpeeds[cpuSpeed]);
    }
}

void CeDImu::DecreaseEmulationSpeed()
{
    if(cpuSpeed > 0)
    {
        cpuSpeed--;
        if(cdi.board)
            cdi.board->cpu.SetEmulationSpeed(cpuSpeeds[cpuSpeed]);
    }
}

void CeDImu::StartGameThread()
{
    if(cdi.board)
        cdi.board->cpu.Run(true);
}

void CeDImu::StopGameThread()
{
    if(cdi.board)
        cdi.board->cpu.Stop();
}
