#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <wx/defs.h>

enum
{
    IDMainFrameOnOpenDisc = wxID_HIGHEST + 1,
    IDMainFrameOnCloseDisc,
    IDMainFrameOnPause,
    IDMainFrameOnExecuteXInstructions,
    IDMainFrameOnReset,
    IDMainFrameOnRebootCore,
    IDMainFrameOnResizeView,
    IDMainFrameOnExportFiles,
    IDMainFrameOnExportAudio,
    IDMainFrameOnExportVideo,
    IDMainFrameOnVDSCViewer,
    IDMainFrameOnCPUViewer,
    IDMainFrameOnRAMSearch,
    IDMainFrameOnDebug,
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
