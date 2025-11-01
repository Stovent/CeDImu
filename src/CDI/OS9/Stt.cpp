#include "Stt.hpp"

namespace OS9
{

std::string statServiceRequestToString(const uint16_t stt)
{
    switch(stt)
    {
    case StatServiceRequest::SS_Opt: return "SS_Opt";
    case StatServiceRequest::SS_Size: return "SS_Size";
    // case GetStatServiceRequest::SS_Reset: return "SS_Reset";
    // case GetStatServiceRequest::SS_WTrk: return "SS_WTrk";
    // case GetStatServiceRequest::SS_Link: return "SS_Link";
    // case GetStatServiceRequest::SS_ULink: return "SS_ULink";
    // case GetStatServiceRequest::SS_Feed: return "SS_Feed";
    // case GetStatServiceRequest::SS_Frz: return "SS_Frz";
    // case GetStatServiceRequest::SS_SPT: return "SS_SPT";
    // case GetStatServiceRequest::SS_SQD: return "SS_SQD";
    // case GetStatServiceRequest::SS_DCmd: return "SS_DCmd";
    // case GetStatServiceRequest::SS_DevNm: return "SS_DevNm";
    case StatServiceRequest::SS_FD: return "SS_FD";
    // case GetStatServiceRequest::SS_Ticks: return "SS_Ticks";
    // case GetStatServiceRequest::SS_Lock: return "SS_Lock";
    // case GetStatServiceRequest::SS_DStat: return "SS_DStat";
    // case GetStatServiceRequest::SS_Joy: return "SS_Joy";
    // case GetStatServiceRequest::SS_BlkRd: return "SS_BlkRd";
    // case GetStatServiceRequest::SS_BlkWr: return "SS_BlkWr";
    // case GetStatServiceRequest::SS_Reten: return "SS_Reten";
    // case GetStatServiceRequest::SS_WFM: return "SS_WFM";
    // case GetStatServiceRequest::SS_RFM: return "SS_RFM";
    // case GetStatServiceRequest::SS_ELog: return "SS_ELog";
    // case GetStatServiceRequest::SS_SSig: return "SS_SSig";
    // case GetStatServiceRequest::SS_Relea: return "SS_Relea";
    // case GetStatServiceRequest::SS_Attr: return "SS_Attr";
    // case GetStatServiceRequest::SS_Break: return "SS_Break";
    // case GetStatServiceRequest::SS_RsBit: return "SS_RsBit";
    // case GetStatServiceRequest::SS_RMS: return "SS_RMS";
    // case GetStatServiceRequest::SS_ACRTC: return "SS_ACRTC";
    // case GetStatServiceRequest::SS_IFC: return "SS_IFC";
    // case GetStatServiceRequest::SS_OFC: return "SS_OFC";
    // case GetStatServiceRequest::SS_EnRTS: return "SS_EnRTS";
    // case GetStatServiceRequest::SS_DsRTS: return "SS_DsRTS";
    // case GetStatServiceRequest::SS_DCOn: return "SS_DCOn";
    // case GetStatServiceRequest::SS_DCOff: return "SS_DCOff";
    // case GetStatServiceRequest::SS_Skip: return "SS_Skip";
    // case GetStatServiceRequest::SS_Mode: return "SS_Mode";
    // case GetStatServiceRequest::SS_Open: return "SS_Open";
    // case GetStatServiceRequest::SS_Close: return "SS_Close";

    case StatServiceRequest::SS_SM: return "SS_SM";
    case StatServiceRequest::SS_SD: return "SS_SD";
    case StatServiceRequest::SS_SC: return "SS_SC";

    // case GetStatServiceRequest::SS_SEvent: return "SS_SEvent";
    // case GetStatServiceRequest::SS_Sound: return "SS_Sound";
    // case GetStatServiceRequest::SS_DSize: return "SS_DSize";
    // case GetStatServiceRequest::SS_Net: return "SS_Net";
    // case GetStatServiceRequest::SS_Rename: return "SS_Rename";
    // case GetStatServiceRequest::SS_Free: return "SS_Free";
    // case GetStatServiceRequest::SS_VarSect: return "SS_VarSect";

    case StatServiceRequest::SS_UCM: return "SS_UCM";
    case StatServiceRequest::SS_DM: return "SS_DM";
    case StatServiceRequest::SS_GC: return "SS_GC";
    case StatServiceRequest::SS_RG: return "SS_RG";
    case StatServiceRequest::SS_DP: return "SS_DP";
    case StatServiceRequest::SS_DR: return "SS_DR";
    case StatServiceRequest::SS_DC: return "SS_DC";
    case StatServiceRequest::SS_CO: return "SS_CO";
    case StatServiceRequest::SS_VIQ: return "SS_VIQ";
    case StatServiceRequest::SS_PT: return "SS_PT";
    case StatServiceRequest::SS_SLink: return "SS_SLink";
    case StatServiceRequest::SS_KB: return "SS_KB";
    case StatServiceRequest::SS_SL: return "SS_SL";

    // case GetStatServiceRequest::SS_Bind: return "SS_Bind";
    // case GetStatServiceRequest::SS_Listen: return "SS_Listen";
    // case GetStatServiceRequest::SS_Connect: return "SS_Connect";
    // case GetStatServiceRequest::SS_Resv: return "SS_Resv";
    // case GetStatServiceRequest::SS_Accept: return "SS_Accept";
    // case GetStatServiceRequest::SS_Recv: return "SS_Recv";
    // case GetStatServiceRequest::SS_Send: return "SS_Send";
    // case GetStatServiceRequest::SS_GNam: return "SS_GNam";
    // case GetStatServiceRequest::SS_SOpt: return "SS_SOpt";
    // case GetStatServiceRequest::SS_GOpt: return "SS_GOpt";
    // case GetStatServiceRequest::SS_Shut: return "SS_Shut";
    // case GetStatServiceRequest::SS_SendTo: return "SS_SendTo";
    // case GetStatServiceRequest::SS_RecvFr: return "SS_RecvFr";
    // case GetStatServiceRequest::SS_Install: return "SS_Install";

    // case GetStatServiceRequest::SS_PCmd: return "SS_PCmd";

    // case GetStatServiceRequest::SS_SN: return "SS_SN";
    // case GetStatServiceRequest::SS_AR: return "SS_AR";
    // case GetStatServiceRequest::SS_MS: return "SS_MS";
    // case GetStatServiceRequest::SS_AC: return "SS_AC";
    // case GetStatServiceRequest::SS_FG: return "SS_FG";
    // case GetStatServiceRequest::SS_Sony: return "SS_Sony";

    default: return "Unknown function " + std::to_string(stt);
    }
}

std::string getStatServiceRequestToString(const uint16_t stt)
{
    switch(stt)
    {
    case GetStatServiceRequest::SS_Ready: return "SS_Ready";
    case GetStatServiceRequest::SS_Pos: return "SS_Pos";
    case GetStatServiceRequest::SS_EOF: return "SS_EOF";
    // case GetStatServiceRequest::SS_Link: return "SS_Link";
    // case GetStatServiceRequest::SS_ULink: return "SS_ULink";
    // case GetStatServiceRequest::SS_Feed: return "SS_Feed";
    // case GetStatServiceRequest::SS_Frz: return "SS_Frz";
    // case GetStatServiceRequest::SS_SPT: return "SS_SPT";
    // case GetStatServiceRequest::SS_SQD: return "SS_SQD";
    // case GetStatServiceRequest::SS_DCmd: return "SS_DCmd";
    case GetStatServiceRequest::SS_DevNm: return "SS_DevNm";
    // case GetStatServiceRequest::SS_DStat: return "SS_DStat";
    // case GetStatServiceRequest::SS_Joy: return "SS_Joy";
    // case GetStatServiceRequest::SS_BlkRd: return "SS_BlkRd";
    // case GetStatServiceRequest::SS_BlkWr: return "SS_BlkWr";
    // case GetStatServiceRequest::SS_Reten: return "SS_Reten";
    // case GetStatServiceRequest::SS_WFM: return "SS_WFM";
    // case GetStatServiceRequest::SS_RFM: return "SS_RFM";
    // case GetStatServiceRequest::SS_ELog: return "SS_ELog";
    // case GetStatServiceRequest::SS_SSig: return "SS_SSig";
    // case GetStatServiceRequest::SS_Relea: return "SS_Relea";
    // case GetStatServiceRequest::SS_Attr: return "SS_Attr";
    // case GetStatServiceRequest::SS_Break: return "SS_Break";
    // case GetStatServiceRequest::SS_RsBit: return "SS_RsBit";
    // case GetStatServiceRequest::SS_RMS: return "SS_RMS";
    case GetStatServiceRequest::SS_FDInf: return "SS_FDInf";
    // case GetStatServiceRequest::SS_ACRTC: return "SS_ACRTC";
    // case GetStatServiceRequest::SS_IFC: return "SS_IFC";
    // case GetStatServiceRequest::SS_OFC: return "SS_OFC";
    // case GetStatServiceRequest::SS_EnRTS: return "SS_EnRTS";
    // case GetStatServiceRequest::SS_DsRTS: return "SS_DsRTS";
    // case GetStatServiceRequest::SS_DCOn: return "SS_DCOn";
    // case GetStatServiceRequest::SS_DCOff: return "SS_DCOff";
    // case GetStatServiceRequest::SS_Skip: return "SS_Skip";
    // case GetStatServiceRequest::SS_Mode: return "SS_Mode";

    case GetStatServiceRequest::SS_Path: return "SS_Path";
    // case GetStatServiceRequest::SS_HEADER: return "SS_HEADER";

    // case GetStatServiceRequest::SS_SEvent: return "SS_SEvent";
    // case GetStatServiceRequest::SS_Sound: return "SS_Sound";
    // case GetStatServiceRequest::SS_DSize: return "SS_DSize";
    // case GetStatServiceRequest::SS_Net: return "SS_Net";
    // case GetStatServiceRequest::SS_Rename: return "SS_Rename";
    case GetStatServiceRequest::SS_Free: return "SS_Free";
    case GetStatServiceRequest::SS_VarSect: return "SS_VarSect";

    // case GetStatServiceRequest::SS_UCM: return "SS_UCM";
    // case GetStatServiceRequest::SS_DM: return "SS_DM";
    // case GetStatServiceRequest::SS_GC: return "SS_GC";
    // case GetStatServiceRequest::SS_RG: return "SS_RG";
    // case GetStatServiceRequest::SS_DP: return "SS_DP";
    // case GetStatServiceRequest::SS_DR: return "SS_DR";
    // case GetStatServiceRequest::SS_DC: return "SS_DC";
    // case GetStatServiceRequest::SS_CO: return "SS_CO";
    // case GetStatServiceRequest::SS_VIQ: return "SS_VIQ";
    // case GetStatServiceRequest::SS_PT: return "SS_PT";
    // case GetStatServiceRequest::SS_SLink: return "SS_SLink";
    // case GetStatServiceRequest::SS_KB: return "SS_KB";
    // case GetStatServiceRequest::SS_SL: return "SS_SL";

    // case GetStatServiceRequest::SS_Bind: return "SS_Bind";
    // case GetStatServiceRequest::SS_Listen: return "SS_Listen";
    // case GetStatServiceRequest::SS_Connect: return "SS_Connect";
    // case GetStatServiceRequest::SS_Resv: return "SS_Resv";
    // case GetStatServiceRequest::SS_Accept: return "SS_Accept";
    // case GetStatServiceRequest::SS_Recv: return "SS_Recv";
    // case GetStatServiceRequest::SS_Send: return "SS_Send";
    // case GetStatServiceRequest::SS_GNam: return "SS_GNam";
    // case GetStatServiceRequest::SS_SOpt: return "SS_SOpt";
    // case GetStatServiceRequest::SS_GOpt: return "SS_GOpt";
    // case GetStatServiceRequest::SS_Shut: return "SS_Shut";
    // case GetStatServiceRequest::SS_SendTo: return "SS_SendTo";
    // case GetStatServiceRequest::SS_RecvFr: return "SS_RecvFr";
    // case GetStatServiceRequest::SS_Install: return "SS_Install";

    // case GetStatServiceRequest::SS_PCmd: return "SS_PCmd";

    // case GetStatServiceRequest::SS_SN: return "SS_SN";
    // case GetStatServiceRequest::SS_AR: return "SS_AR";
    // case GetStatServiceRequest::SS_MS: return "SS_MS";
    // case GetStatServiceRequest::SS_AC: return "SS_AC";
    case GetStatServiceRequest::SS_CDFD: return "SS_CDFD";
    // case GetStatServiceRequest::SS_FG: return "SS_FG";
    // case GetStatServiceRequest::SS_Sony: return "SS_Sony";

    default: return statServiceRequestToString(stt);
    }
}

std::string setStatServiceRequestToString(const uint16_t stt)
{
    switch(stt)
    {
    case SetStatServiceRequest::SS_Reset: return "SS_Reset";
    case SetStatServiceRequest::SS_WTrk: return "SS_WTrk";
    // case SetStatServiceRequest::SS_EOF: return "SS_EOF";
    // case SetStatServiceRequest::SS_Link: return "SS_Link";
    // case SetStatServiceRequest::SS_ULink: return "SS_ULink";
    case SetStatServiceRequest::SS_Feed: return "SS_Feed";
    // case SetStatServiceRequest::SS_Frz: return "SS_Frz";
    // case SetStatServiceRequest::SS_SPT: return "SS_SPT";
    // case SetStatServiceRequest::SS_SQD: return "SS_SQD";
    // case SetStatServiceRequest::SS_DCmd: return "SS_DCmd";
    case SetStatServiceRequest::SS_Ticks: return "SS_Ticks";
    case SetStatServiceRequest::SS_Lock: return "SS_Lock";
    // case SetStatServiceRequest::SS_DStat: return "SS_DStat";
    // case SetStatServiceRequest::SS_Joy: return "SS_Joy";
    // case SetStatServiceRequest::SS_BlkRd: return "SS_BlkRd";
    // case SetStatServiceRequest::SS_BlkWr: return "SS_BlkWr";
    // case SetStatServiceRequest::SS_Reten: return "SS_Reten";
    case SetStatServiceRequest::SS_WFM: return "SS_WFM";
    case SetStatServiceRequest::SS_RFM: return "SS_RFM";
    // case SetStatServiceRequest::SS_ELog: return "SS_ELog";
    case SetStatServiceRequest::SS_SSig: return "SS_SSig";
    case SetStatServiceRequest::SS_Relea: return "SS_Relea";
    case SetStatServiceRequest::SS_Attr: return "SS_Attr";
    // case SetStatServiceRequest::SS_Break: return "SS_Break";
    // case SetStatServiceRequest::SS_RsBit: return "SS_RsBit";
    // case SetStatServiceRequest::SS_RMS: return "SS_RMS";
    // case SetStatServiceRequest::SS_ACRTC: return "SS_ACRTC";
    // case SetStatServiceRequest::SS_IFC: return "SS_IFC";
    // case SetStatServiceRequest::SS_OFC: return "SS_OFC";
    case SetStatServiceRequest::SS_EnRTS: return "SS_EnRTS";
    case SetStatServiceRequest::SS_DsRTS: return "SS_DsRTS";
    case SetStatServiceRequest::SS_DCOn: return "SS_DCOn";
    case SetStatServiceRequest::SS_DCOff: return "SS_DCOff";
    case SetStatServiceRequest::SS_Skip: return "SS_Skip";
    // case SetStatServiceRequest::SS_Mode: return "SS_Mode";
    case SetStatServiceRequest::SS_Open: return "SS_Open";
    case SetStatServiceRequest::SS_Close: return "SS_Close";

    case SetStatServiceRequest::SS_Play: return "SS_Play";
    case SetStatServiceRequest::SS_Raw: return "SS_Raw";
    case SetStatServiceRequest::SS_Seek: return "SS_Seek";
    case SetStatServiceRequest::SS_Abort: return "SS_Abort";
    case SetStatServiceRequest::SS_CDDA: return "SS_CDDA";
    case SetStatServiceRequest::SS_Pause: return "SS_Pause";
    case SetStatServiceRequest::SS_Eject: return "SS_Eject";
    case SetStatServiceRequest::SS_Mount: return "SS_Mount";
    case SetStatServiceRequest::SS_Stop: return "SS_Stop";
    case SetStatServiceRequest::SS_Cont: return "SS_Cont";
    case SetStatServiceRequest::SS_Disable: return "SS_Disable";
    case SetStatServiceRequest::SS_Enable: return "SS_Enable";
    case SetStatServiceRequest::SS_ReadToc: return "SS_ReadToc";

    // case SetStatServiceRequest::SS_SEvent: return "SS_SEvent";
    // case SetStatServiceRequest::SS_Sound: return "SS_Sound";
    // case SetStatServiceRequest::SS_DSize: return "SS_DSize";
    // case SetStatServiceRequest::SS_Net: return "SS_Net";
    // case SetStatServiceRequest::SS_Rename: return "SS_Rename";
    // case SetStatServiceRequest::SS_Free: return "SS_Free";
    // case SetStatServiceRequest::SS_VarSect: return "SS_VarSect";

    // case SetStatServiceRequest::SS_UCM: return "SS_UCM";
    // case SetStatServiceRequest::SS_DM: return "SS_DM";
    // case SetStatServiceRequest::SS_GC: return "SS_GC";
    // case SetStatServiceRequest::SS_RG: return "SS_RG";
    // case SetStatServiceRequest::SS_DP: return "SS_DP";
    // case SetStatServiceRequest::SS_DR: return "SS_DR";
    // case SetStatServiceRequest::SS_DC: return "SS_DC";
    // case SetStatServiceRequest::SS_CO: return "SS_CO";
    // case SetStatServiceRequest::SS_VIQ: return "SS_VIQ";
    // case SetStatServiceRequest::SS_PT: return "SS_PT";
    // case SetStatServiceRequest::SS_SLink: return "SS_SLink";
    // case SetStatServiceRequest::SS_KB: return "SS_KB";
    // case SetStatServiceRequest::SS_SL: return "SS_SL";

    // case SetStatServiceRequest::SS_Bind: return "SS_Bind";
    // case SetStatServiceRequest::SS_Listen: return "SS_Listen";
    // case SetStatServiceRequest::SS_Connect: return "SS_Connect";
    // case SetStatServiceRequest::SS_Resv: return "SS_Resv";
    // case SetStatServiceRequest::SS_Accept: return "SS_Accept";
    // case SetStatServiceRequest::SS_Recv: return "SS_Recv";
    // case SetStatServiceRequest::SS_Send: return "SS_Send";
    // case SetStatServiceRequest::SS_GNam: return "SS_GNam";
    // case SetStatServiceRequest::SS_SOpt: return "SS_SOpt";
    // case SetStatServiceRequest::SS_GOpt: return "SS_GOpt";
    // case SetStatServiceRequest::SS_Shut: return "SS_Shut";
    // case SetStatServiceRequest::SS_SendTo: return "SS_SendTo";
    // case SetStatServiceRequest::SS_RecvFr: return "SS_RecvFr";
    // case SetStatServiceRequest::SS_Install: return "SS_Install";

    // case SetStatServiceRequest::SS_PCmd: return "SS_PCmd";

    // case SetStatServiceRequest::SS_SN: return "SS_SN";
    // case SetStatServiceRequest::SS_AR: return "SS_AR";
    // case SetStatServiceRequest::SS_MS: return "SS_MS";
    // case SetStatServiceRequest::SS_AC: return "SS_AC";
    case SetStatServiceRequest::SS_CCHAN: return "SS_CCHAN";
    // case SetStatServiceRequest::SS_FG: return "SS_FG";
    // case SetStatServiceRequest::SS_Sony: return "SS_Sony";

    default: return statServiceRequestToString(stt);
    }
}

std::string subGetStatServiceRequestToString(uint16_t stt, uint16_t substt)
{
    switch(stt)
    {
    case SS_SM:
        switch(substt)
        {
        case SM_Info: return "SM_Info";
        case SM_Stat: return "SM_Stat";
        default: return "Unknown SS_SM subStat " + std::to_string(substt);
        }

    case SS_VIQ:
        switch(substt)
        {
        case VIQ_TxtL: return "VIQ_TxtL";
        case VIQ_CPos: return "VIQ_CPos";
        case VIQ_JCPs: return "VIQ_JCPs";
        case VIQ_FDta: return "VIQ_FDta";
        case VIQ_GDta: return "VIQ_GDta";
        // case VIQ_RGInfo: return "VIQ_RGInfo";
        case VIQ_PntR: return "VIQ_PntR";
        case VIQ_RLoc: return "VIQ_RLoc";
        // case VIQ_DMInfo: return "VIQ_DMInfo";
        default: return "Unknown SS_VIQ subStat " + std::to_string(substt);
        }

    case SS_PT:
        switch(substt)
        {
        case PT_Coord: return "PT_Coord";
        default: return "Unknown SS_PT subStat " + std::to_string(substt);
        }

    case SS_KB:
        switch(substt)
        {
        case KB_Rdy: return "KB_Rdy";
        case KB_Read: return "KB_Read";
        case KB_Stat: return "KB_Stat";
        // case KB_Avail: return "KB_Avail";
        // case KB_NrAvail: return "KB_NrAvail";
        default: return "Unknown SS_KB subStat " + std::to_string(substt);
        }

    default: return "Unknown stat " + std::to_string(stt);
    }
}

std::string subSetStatServiceRequestToString(uint16_t stt, uint16_t substt)
{
    switch(stt)
    {
    case SS_SM:
        switch(substt)
        {
        case SM_Creat: return "SM_Creat";
        case SM_Out: return "SM_Out";
        case SM_Off: return "SM_Off";
        case SM_Cncl: return "SM_Cncl";
        case SM_Close: return "SM_Close";
        default: return "Unknown SS_SM subStat " + std::to_string(substt);
        }

    case SS_SD:
        switch(substt)
        {
        case SD_MMix: return "SD_MMix";
        case SD_SMix: return "SD_SMix";
        case SD_Loop: return "SD_Loop";
        default: return "Unknown SS_SD subStat " + std::to_string(substt);
        }

    case SS_SC:
        switch(substt)
        {
        case SC_Atten: return "SC_Atten";
        default: return "Unknown SS_SC subStat " + std::to_string(substt);
        }

    case SS_DM:
        switch(substt)
        {
        case DM_Creat: return "DM_Creat";
        case DM_Org: return "DM_Org";
        case DM_Copy: return "DM_Copy";
        case DM_Exch: return "DM_Exch";
        case DM_TCpy: return "DM_TCpy";
        case DM_TExc: return "DM_TExc";
        case DM_Write: return "DM_Write";
        case DM_IrWr: return "DM_IrWr";
        case DM_Read: return "DM_Read";
        case DM_WrPix: return "DM_WrPix";
        case DM_RdPix: return "DM_RdPix";
        case DM_Cncl: return "DM_Cncl";
        case DM_Close: return "DM_Close";
        case DM_DMDup: return "DM_DMDup";
        default: return "Unknown SS_DM subStat " + std::to_string(substt);
        }

    case SS_GC:
        switch(substt)
        {
        case GC_Pos: return "GC_Pos";
        case GC_Show: return "GC_Show";
        case GC_Hide: return "GC_Hide";
        case GC_Ptn: return "GC_Ptn";
        case GC_Col: return "GC_Col";
        case GC_Blnk: return "GC_Blnk";
        case GC_Org: return "GC_Org";
        default: return "Unknown SS_GC subStat " + std::to_string(substt);
        }

    case SS_RG:
        switch(substt)
        {
        case RG_Creat: return "RG_Creat";
        case RG_Isect: return "RG_Isect";
        case RG_Union: return "RG_Union";
        case RG_Diff: return "RG_Diff";
        case RG_XOR: return "RG_XOR";
        case RG_Move: return "RG_Move";
        case RG_Del: return "RG_Del";
        default: return "Unknown SS_RG subStat " + std::to_string(substt);
        }

    case SS_DP:
        switch(substt)
        {
        case DP_Ptn: return "DP_Ptn";
        case DP_PAln: return "DP_PAln";
        case DP_SCMM: return "DP_SCMM";
        case DP_SCR: return "DP_SCR";
        case DP_GFnt: return "DP_GFnt";
        case DP_AFnt: return "DP_AFnt";
        case DP_DFnt: return "DP_DFnt";
        case DP_RFnt: return "DP_RFnt";
        case DP_Clip: return "DP_Clip";
        case DP_PnSz: return "DP_PnSz";
        case DP_PStyl: return "DP_PStyl";
        case DP_TCol: return "DP_TCol";
        default: return "Unknown SS_DP subStat " + std::to_string(substt);
        }

    case SS_DR:
        switch(substt)
        {
        case DR_Dot: return "DR_Dot";
        case DR_Line: return "DR_Line";
        case DR_PLin: return "DR_PLin";
        case DR_CArc: return "DR_CArc";
        case DR_EArc: return "DR_EArc";
        case DR_Rect: return "DR_Rect";
        case DR_ERect: return "DR_ERect";
        case DR_PGon: return "DR_PGon";
        case DR_Circ: return "DR_Circ";
        case DR_CWdg: return "DR_CWdg";
        case DR_Elps: return "DR_Elps";
        case DR_EWdg: return "DR_EWdg";
        case DR_DRgn: return "DR_DRgn";
        case DR_BFil: return "DR_BFil";
        case DR_FFil: return "DR_FFil";
        case DR_Copy: return "DR_Copy";
        case DR_Text: return "DR_Text";
        case DR_JTxt: return "DR_JTxt";
        default: return "Unknown SS_DR subStat " + std::to_string(substt);
        }

    case SS_DC:
        switch(substt)
        {
        case DC_CrFCT: return "DC_CrFCT";
        case DC_RdFCT: return "DC_RdFCT";
        case DC_WrFCT: return "DC_WrFCT";
        case DC_RdFI: return "DC_RdFI";
        case DC_WrFI: return "DC_WrFI";
        case DC_DlFCT: return "DC_DlFCT";
        case DC_CrLCT: return "DC_CrLCT";
        case DC_RdLCT: return "DC_RdLCT";
        case DC_WrLCT: return "DC_WrLCT";
        case DC_RdLI: return "DC_RdLI";
        case DC_WrLI: return "DC_WrLI";
        case DC_DlLCT: return "DC_DlLCT";
        case DC_FLnk: return "DC_FLnk";
        case DC_LLnk: return "DC_LLnk";
        case DC_Exec: return "DC_Exec";
        case DC_Intl: return "DC_Intl";
        case DC_NOP: return "DC_NOP";
        case DC_SSig: return "DC_SSig";
        case DC_Relea: return "DC_Relea";
        case DC_SetCmp: return "DC_SetCmp";
        case DC_PRdLCT: return "DC_PRdLCT";
        case DC_PWrLCT: return "DC_PWrLCT";
        // case DC_SetAR: return "DC_SetAR";
        default: return "Unknown SS_DC subStat " + std::to_string(substt);
        }

    case SS_CO:
        switch(substt)
        {
        case CO_COD: return "CO_COD";
        case CO_SCMM: return "CO_SCMM";
        case CO_AFnt: return "CO_AFnt";
        case CO_DFnt: return "CO_DFnt";
        default: return "Unknown SS_CO subStat " + std::to_string(substt);
        }

    case SS_PT:
        switch(substt)
        {
        case PT_SSig: return "PT_SSig";
        case PT_Relea: return "PT_Relea";
        case PT_Pos: return "PT_Pos";
        case PT_Org: return "PT_Org";
        default: return "Unknown SS_PT subStat " + std::to_string(substt);
        }

    case SS_KB:
        switch(substt)
        {
        case KB_Rel: return "KB_Rel";
        case KB_Repeat: return "KB_Repeat";
        case KB_SSig: return "KB_SSig";
        default: return "Unknown SS_KB subStat " + std::to_string(substt);
        }

    default: return "Unknown stat " + std::to_string(stt);
    }
}

} // namespace OS9
