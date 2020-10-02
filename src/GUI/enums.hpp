#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <wx/defs.h>

enum
{
    IDMainFrameOnOpenROM = wxID_HIGHEST + 1,
    IDMainFrameOnLoadBIOS,
    IDMainFrameOnCloseROM,
    IDMainFrameOnPause,
    IDMainFrameOnExecuteXInstructions,
    IDMainFrameOnRebootCore,
    IDMainFrameOnExportFiles,
    IDMainFrameOnExportAudio,
    IDMainFrameOnExportVideo,
    IDMainFrameOnSlaveViewer,
    IDMainFrameOnVDSCViewer,
    IDMainFrameOnDisassembler,
    IDMainFrameOnRAMSearch,
    IDMainFrameOnSettings,
    IDMainFrameOnAbout,

    IDDisassemblerOnClose,
    IDDisassemblerpc, IDDisassemblersr,

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
