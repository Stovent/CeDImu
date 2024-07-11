#ifndef GUI_ENUMS_HPP
#define GUI_ENUMS_HPP

#include <wx/defs.h>

enum
{
    IDMainFrameOnOpenDisc = wxID_HIGHEST + 1,
    IDMainFrameOnCloseDisc,
    IDMainFrameOnScreenshot,
    IDMainFrameOnPause,
    IDMainFrameOnSingleStep,
    IDMainFrameOnFrameAdvance,
    IDMainFrameOnIncreaseSpeed,
    IDMainFrameOnDecreaseSpeed,
    IDMainFrameOnReloadCore,
    IDMainFrameOnResizeView,
    IDMainFrameOnExportAudio,
    IDMainFrameOnExportFiles,
    IDMainFrameOnExportVideo,
    IDMainFrameOnExportRawVideo,
    IDMainFrameOnCPUViewer,
    IDMainFrameOnOS9Viewer,
    IDMainFrameOnVDSCViewer,
    IDMainFrameOnDebugFrame,
    IDMainFrameOnSettings,
    IDSettingsFrameOnNewConfig,
    IDSettingsFrameOnDeleteConfig,
    IDSettingsFrameOnSelectBios,
    IDMainFrameBiosMenuBaseIndex, // Must always be the last in this list.
};

#endif // GUI_ENUMS_HPP
