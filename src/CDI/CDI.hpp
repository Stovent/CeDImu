#ifndef CDI_HPP
#define CDI_HPP

class CDI;

#include <string>

#include "CDIDisk.hpp"
#include "CDIFile.hpp"
#include "CDIDirectory.hpp"
#include "../CeDImu.hpp"

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
