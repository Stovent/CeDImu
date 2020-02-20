#ifndef CDI_HPP
#define CDI_HPP

class CDI;

#include <string>

#include "CDIDisk.hpp"
#include "CDIFile.hpp"
#include "CDIDirectory.hpp"
#include "../CeDImu.hpp"

enum AudioCodingInformation
{
    emphasis = 0b01000000,
    bps      = 0b00110000, // bits per sample
    sf       = 0b00001100, // sampling frequency
    ms       = 0b00000011  // mono/stereo
};

class CDI
{
    CDIDisk disk;

    void LoadCDIFileSystem();

public:
    CeDImu* cedimu;
    CDIFile* mainModule;
    CDIDirectory rootDirectory;

    std::string gameName;
    std::string romPath;
    std::string gameFolder; // romPath + gameName + "/"

    CDI(CeDImu* app);
    ~CDI();

    bool OpenROM(const std::string rom);
    void CloseROM();
    bool CreateSubfoldersFromROMDirectory(std::string path = "");
    bool LoadModuleInMemory(std::string moduleName, uint32_t address);

    bool ExportAudio();
    void ExportAudioInfo();
    bool ExportFiles();
    void ExportFilesInfo();
    void ExportSectorsInfo();
};

#endif // CDI_HPP
