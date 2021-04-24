#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <wx/defs.h>

enum
{
    IDMainFrameOnOpenROM = wxID_HIGHEST + 1,
    IDMainFrameOnCloseROM,
    IDMainFrameOnPause,
    IDMainFrameOnExecuteXInstructions,
    IDMainFrameOnReset,
    IDMainFrameOnRebootCore,
    IDMainFrameOnExportFiles,
    IDMainFrameOnExportAudio,
    IDMainFrameOnExportVideo,
    IDMainFrameOnVDSCViewer,
    IDMainFrameOnCPUViewer,
    IDMainFrameOnRAMSearch,
    IDMainFrameOnSettings,
    IDMainFrameOnAbout,

    IDCPUViewerOnClose,
    IDCPUViewerTimer,
    IDCPUViewerpc, IDCPUViewersr,

    IDRAMSearchListCheckMisaligned,
    IDRAMSearchListSigned,
    IDRAMSearchListUnsigned,
    IDRAMSearchListHexadecimal,
    IDRAMSearchListByte1,
    IDRAMSearchListByte2,
    IDRAMSearchListByte4,

    IDVDSCViewerTimer,
};

#endif // ENUMS_HPP
