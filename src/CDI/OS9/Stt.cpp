#include "Stt.hpp"

namespace OS9
{

std::string sttFunctionToString(const SttFunction stt)
{
    switch(stt)
    {
    case SttFunction::SS_Opt: return "SS_Opt";
    case SttFunction::SS_Ready: return "SS_Ready";
    case SttFunction::SS_Size: return "SS_Size";
    case SttFunction::SS_Reset: return "SS_Reset";
    case SttFunction::SS_WTrk: return "SS_WTrk";
    case SttFunction::SS_Pos: return "SS_Pos";
    case SttFunction::SS_EOF: return "SS_EOF";
    case SttFunction::SS_Link: return "SS_Link";
    case SttFunction::SS_ULink: return "SS_ULink";
    case SttFunction::SS_Feed: return "SS_Feed";
    case SttFunction::SS_Frz: return "SS_Frz";
    case SttFunction::SS_SPT: return "SS_SPT";
    case SttFunction::SS_SQD: return "SS_SQD";
    case SttFunction::SS_DCmd: return "SS_DCmd";
    case SttFunction::SS_DevNm: return "SS_DevNm";
    case SttFunction::SS_FD: return "SS_FD";
    case SttFunction::SS_Ticks: return "SS_Ticks";
    case SttFunction::SS_Lock: return "SS_Lock";
    case SttFunction::SS_DStat: return "SS_DStat";
    case SttFunction::SS_Joy: return "SS_Joy";
    case SttFunction::SS_BlkRd: return "SS_BlkRd";
    case SttFunction::SS_BlkWr: return "SS_BlkWr";
    case SttFunction::SS_Reten: return "SS_Reten";
    case SttFunction::SS_WFM: return "SS_WFM";
    case SttFunction::SS_RFM: return "SS_RFM";
    case SttFunction::SS_ELog: return "SS_ELog";
    case SttFunction::SS_SSig: return "SS_SSig";
    case SttFunction::SS_Relea: return "SS_Relea";
    case SttFunction::SS_Attr: return "SS_Attr";
    case SttFunction::SS_Break: return "SS_Break";
    case SttFunction::SS_RsBit: return "SS_RsBit";
    case SttFunction::SS_RMS: return "SS_RMS";
    case SttFunction::SS_FDInf: return "SS_FDInf";
    case SttFunction::SS_ACRTC: return "SS_ACRTC";
    case SttFunction::SS_IFC: return "SS_IFC";
    case SttFunction::SS_OFC: return "SS_OFC";
    case SttFunction::SS_EnRTS: return "SS_EnRTS";
    case SttFunction::SS_DsRTS: return "SS_DsRTS";
    case SttFunction::SS_DCOn: return "SS_DCOn";
    case SttFunction::SS_DCOff: return "SS_DCOff";
    case SttFunction::SS_Skip: return "SS_Skip";
    case SttFunction::SS_Mode: return "SS_Mode";
    case SttFunction::SS_Open: return "SS_Open";
    case SttFunction::SS_Close: return "SS_Close";

    case SttFunction::SS_Path: return "SS_Path";
    case SttFunction::SS_Play: return "SS_Play";
    case SttFunction::SS_HEADER: return "SS_HEADER";
    case SttFunction::SS_Raw: return "SS_Raw";
    case SttFunction::SS_Seek: return "SS_Seek";
    case SttFunction::SS_Abort: return "SS_Abort";
    case SttFunction::SS_CDDA: return "SS_CDDA";
    case SttFunction::SS_Pause: return "SS_Pause";
    case SttFunction::SS_Eject: return "SS_Eject";
    case SttFunction::SS_Mount: return "SS_Mount";
    case SttFunction::SS_Stop: return "SS_Stop";
    case SttFunction::SS_Cont: return "SS_Cont";
    case SttFunction::SS_Disable: return "SS_Disable";
    case SttFunction::SS_Enable: return "SS_Enable";
    case SttFunction::SS_ReadToc: return "SS_ReadToc";
    case SttFunction::SS_SM: return "SS_SM";
    case SttFunction::SS_SD: return "SS_SD";
    case SttFunction::SS_SC: return "SS_SC";

    case SttFunction::SS_SEvent: return "SS_SEvent";
    case SttFunction::SS_Sound: return "SS_Sound";
    case SttFunction::SS_DSize: return "SS_DSize";
    case SttFunction::SS_Net: return "SS_Net";
    case SttFunction::SS_Rename: return "SS_Rename";
    case SttFunction::SS_Free: return "SS_Free";
    case SttFunction::SS_VarSect: return "SS_VarSect";

    case SttFunction::SS_UCM: return "SS_UCM";
    case SttFunction::SS_DM: return "SS_DM";
    case SttFunction::SS_GC: return "SS_GC";
    case SttFunction::SS_RG: return "SS_RG";
    case SttFunction::SS_DP: return "SS_DP";
    case SttFunction::SS_DR: return "SS_DR";
    case SttFunction::SS_DC: return "SS_DC";
    case SttFunction::SS_CO: return "SS_CO";
    case SttFunction::SS_VIQ: return "SS_VIQ";
    case SttFunction::SS_PT: return "SS_PT";
    case SttFunction::SS_SLink: return "SS_SLink";
    case SttFunction::SS_KB: return "SS_KB";

    case SttFunction::SS_Bind: return "SS_Bind";
    case SttFunction::SS_Listen: return "SS_Listen";
    case SttFunction::SS_Connect: return "SS_Connect";
    case SttFunction::SS_Resv: return "SS_Resv";
    case SttFunction::SS_Accept: return "SS_Accept";
    case SttFunction::SS_Recv: return "SS_Recv";
    case SttFunction::SS_Send: return "SS_Send";
    case SttFunction::SS_GNam: return "SS_GNam";
    case SttFunction::SS_SOpt: return "SS_SOpt";
    case SttFunction::SS_GOpt: return "SS_GOpt";
    case SttFunction::SS_Shut: return "SS_Shut";
    case SttFunction::SS_SendTo: return "SS_SendTo";
    case SttFunction::SS_RecvFr: return "SS_RecvFr";
    case SttFunction::SS_Install: return "SS_Install";

    case SttFunction::SS_PCmd: return "SS_PCmd";

    case SttFunction::SS_SN: return "SS_SN";
    case SttFunction::SS_AR: return "SS_AR";
    case SttFunction::SS_MS: return "SS_MS";
    case SttFunction::SS_AC: return "SS_AC";
    case SttFunction::SS_CDFD: return "SS_CDFD";
    case SttFunction::SS_CCHAN: return "SS_CCHAN";
    default: return "Unknown function " + std::to_string((int)stt);
    }
}

} // namespace OS9
