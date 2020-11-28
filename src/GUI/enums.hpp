#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <wx/defs.h>

enum
{
    IDMainFrameOnOpenROM = wxID_HIGHEST + 1,
    IDMainFrameOnCloseROM,
    IDMainFrameOnPause,
    IDMainFrameOnExecuteXInstructions,
    IDMainFrameOnRebootCore,
    IDMainFrameOnExportFiles,
    IDMainFrameOnExportAudio,
    IDMainFrameOnExportVideo,
    IDMainFrameOnSlaveViewer,
    IDMainFrameOnVDSCViewer,
    IDMainFrameOnCPUViewer,
    IDMainFrameOnRAMSearch,
    IDMainFrameOnSettings,
    IDMainFrameOnAbout,

    IDCPUViewerOnClose,
    IDCPUViewerpc, IDCPUViewersr,

    IDRAMSearchListCheckMisaligned,
    IDRAMSearchListSigned,
    IDRAMSearchListUnsigned,
    IDRAMSearchListHexadecimal,
    IDRAMSearchListByte1,
    IDRAMSearchListByte2,
    IDRAMSearchListByte4,

    IDVDSCViewerTimer,
    IDSlaveViewerTimer,
};

#endif // ENUMS_HPP