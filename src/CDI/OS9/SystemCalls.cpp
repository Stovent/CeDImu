#include "SystemCalls.hpp"
#include "Stt.hpp"

#include <cstdio>

namespace OS9
{

#define GETSTR(areg) (get(regs.at(SCC68070::Register::areg)) != nullptr ? (char*)get(regs.at(SCC68070::Register::areg)) : "")
#define REG(reg) regs.at(SCC68070::Register::reg)

std::string eventNameToString(const Event evt)
{
    switch(evt)
    {
    case Event::Ev_Link:  return "Ev$Link";
    case Event::Ev_UnLnk: return "Ev$UnLnk";
    case Event::Ev_Creat: return "Ev$Creat";
    case Event::Ev_Delet: return "Ev$Delet";
    case Event::Ev_Wait:  return "Ev$Wait";
    case Event::Ev_WaitR: return "Ev$WaitR";
    case Event::Ev_Read:  return "Ev$Read";
    case Event::Ev_Info:  return "Ev$Info";
    case Event::Ev_Signl: return "Ev$Signl";
    case Event::Ev_Signl_Ev_All: return "Ev$Signl+Ev$All";
    case Event::Ev_Pulse: return "Ev$Pulse";
    case Event::Ev_Pulse_Ev_All: return "Ev$Pulse+Ev$All";
    case Event::Ev_Set:   return "Ev$Set";
    case Event::Ev_Set_Ev_All: return "Ev$Set+Ev$All";
    case Event::Ev_SetR:  return "Ev$SetR";
    case Event::Ev_SetR_Ev_All: return "Ev$SetR+Ev$All";
    default: return "Unknown event " + std::to_string((int)evt);
    }
}

std::string eventInputsToString(const Event evt, const std::map<SCC68070::Register, uint32_t>& regs, const std::function<const uint8_t*(const uint32_t)>& get)
{
    char event[256]= {0};
    switch(evt)
    {
    case Event::Ev_Link:  snprintf(event, 256, "(a0).s=\"%s\"", GETSTR(A0)); break;
    case Event::Ev_UnLnk: snprintf(event, 256, "d0.l=%d", REG(D0)); break;
    case Event::Ev_Creat: snprintf(event, 256, "(a0).s=\"%s\" d0.l=%d d2.w=%hd d3.w=%hd", GETSTR(A0), REG(D0), REG(D2), REG(D3)); break;
    case Event::Ev_Delet: snprintf(event, 256, "(a0).s=\"%s\"", GETSTR(A0)); break;
    case Event::Ev_Wait:  snprintf(event, 256, "d0.l=%d d2.l=%d d3.l=%d", REG(D0), REG(D2), REG(D3)); break;
    case Event::Ev_WaitR: snprintf(event, 256, "d0.l=%d d2.l=%d d3.l=%d", REG(D0), REG(D2), REG(D3)); break;
    case Event::Ev_Read:  snprintf(event, 256, "d0.l=%d", REG(D0)); break;
    case Event::Ev_Info:  snprintf(event, 256, "a0=0x%X d0.l=%d", REG(A0), REG(D0)); break;
    case Event::Ev_Signl: snprintf(event, 256, "d0.l=%d", REG(D0)); break;
    case Event::Ev_Signl_Ev_All: snprintf(event, 256, "d0.l=%d", REG(D0)); break;
    case Event::Ev_Pulse: snprintf(event, 256, "d0.l=%d d2.l=%d", REG(D0), REG(D2)); break;
    case Event::Ev_Pulse_Ev_All: snprintf(event, 256, "d0.l=%d d2.l=%d", REG(D0), REG(D2)); break;
    case Event::Ev_Set:   snprintf(event, 256, "d0.l=%d d2.l=%d", REG(D0), REG(D2)); break;
    case Event::Ev_Set_Ev_All: snprintf(event, 256, "d0.l=%d d2.l=%d", REG(D0), REG(D2)); break;
    case Event::Ev_SetR:  snprintf(event, 256, "d0.l=%d d2.l=%d", REG(D0), REG(D2)); break;
    case Event::Ev_SetR_Ev_All: snprintf(event, 256, "d0.l=%d d2.l=%d", REG(D0), REG(D2)); break;
    default: snprintf(event, 256, "Unknown event %d", (int)evt); break;
    }
    return event;
}

std::string eventOutputsToString(const Event evt, const std::map<SCC68070::Register, uint32_t>& regs, const std::function<const uint8_t*(const uint32_t)>& get)
{
    char event[256]= {0};
    switch(evt)
    {
    case Event::Ev_Link:  snprintf(event, 256, "(a0).s=\"%s\" d0.l=%d", GETSTR(A0), REG(D0)); break;
    case Event::Ev_UnLnk: return "";
    case Event::Ev_Creat: snprintf(event, 256, "(a0).s=\"%s\" d0.l=%d", GETSTR(A0), REG(D0)); break;
    case Event::Ev_Delet: snprintf(event, 256, "(a0).s=\"%s\"", GETSTR(A0)); break;
    case Event::Ev_Wait:  snprintf(event, 256, "d1.l=%d", REG(D1)); break;
    case Event::Ev_WaitR: snprintf(event, 256, "d1.l=%d d2.l=%d d3.l=%d", REG(D1), REG(D2), REG(D3)); break;
    case Event::Ev_Read:  snprintf(event, 256, "d1.l=%d", REG(D1)); break;
    case Event::Ev_Info:  snprintf(event, 256, "d0.l=%d a0=%d", REG(D0), REG(A0)); break;
    case Event::Ev_Signl: return "";
    case Event::Ev_Signl_Ev_All: return "";
    case Event::Ev_Pulse: return "";
    case Event::Ev_Pulse_Ev_All: return "";
    case Event::Ev_Set:   snprintf(event, 256, "d1.l=%d", REG(D1)); break;
    case Event::Ev_Set_Ev_All: snprintf(event, 256, "d1.l=%d", REG(D1)); break;
    case Event::Ev_SetR:  snprintf(event, 256, "d1.l=%d", REG(D1)); break;
    case Event::Ev_SetR_Ev_All: snprintf(event, 256, "d1.l=%d", REG(D1)); break;
    default: snprintf(event, 256, "Unknown event %d", (int)evt); break;
    }
    return event;
}

std::string systemCallNameToString(const SystemCallType call)
{
    switch(call)
    {
    case SystemCallType::F_Link:    return "F$Link";
    case SystemCallType::F_Load:    return "F$Load";
    case SystemCallType::F_UnLink:  return "F$UnLink";
    case SystemCallType::F_Fork:    return "F$Fork";
    case SystemCallType::F_Wait:    return "F$Wait";
    case SystemCallType::F_Chain:   return "F$Chain";
    case SystemCallType::F_Exit:    return "F$Exit";
    case SystemCallType::F_Mem:     return "F$Mem";
    case SystemCallType::F_Send:    return "F$Send";
    case SystemCallType::F_Icpt:    return "F$Icpt";
    case SystemCallType::F_Sleep:   return "F$Sleep";
    case SystemCallType::F_SSpd:    return "F$SSpd";
    case SystemCallType::F_ID:      return "F$ID";
    case SystemCallType::F_SPrior:  return "F$SPrior";
    case SystemCallType::F_STrap:   return "F$STrap";
    case SystemCallType::F_PErr:    return "F$PErr";
    case SystemCallType::F_PrsNam:  return "F$PrsNam";
    case SystemCallType::F_CmpNam:  return "F$CmpNam";
    case SystemCallType::F_SchBit:  return "F$SchBit";
    case SystemCallType::F_AllBit:  return "F$AllBit";
    case SystemCallType::F_DelBit:  return "F$DelBit";
    case SystemCallType::F_Time:    return "F$Time";
    case SystemCallType::F_STime:   return "F$STime";
    case SystemCallType::F_CRC:     return "F$CRC";
    case SystemCallType::F_GPrDsc:  return "F$GPrDsc";
    case SystemCallType::F_GBlkMp:  return "F$GBlkMp";
    case SystemCallType::F_GModDr:  return "F$GModDr";
    case SystemCallType::F_CpyMem:  return "F$CpyMem";
    case SystemCallType::F_SUser:   return "F$SUser";
    case SystemCallType::F_UnLoad:  return "F$UnLoad";
    case SystemCallType::F_RTE:     return "F$RTE";
    case SystemCallType::F_GPrDBT:  return "F$GPrDBT";
    case SystemCallType::F_Julian:  return "F$Julian";
    case SystemCallType::F_TLink:   return "F$TLink";
    case SystemCallType::F_DFork:   return "F$DFork";
    case SystemCallType::F_DExec:   return "F$DExec";
    case SystemCallType::F_DExit:   return "F$DExit";
    case SystemCallType::F_DatMod:  return "F$DatMod";
    case SystemCallType::F_SetCRC:  return "F$SetCRC";
    case SystemCallType::F_SetSys:  return "F$SetSys";
    case SystemCallType::F_SRqMem:  return "F$SRqMem";
    case SystemCallType::F_SRtMem:  return "F$SRtMem";
    case SystemCallType::F_IRQ:     return "F$IRQ";
    case SystemCallType::F_IOQu:    return "F$IOQu";
    case SystemCallType::F_AProc:   return "F$AProc";
    case SystemCallType::F_NProc:   return "F$NProc";
    case SystemCallType::F_VModul:  return "F$VModul";
    case SystemCallType::F_FindPD:  return "F$FindPD";
    case SystemCallType::F_AllPD:   return "F$AllPD";
    case SystemCallType::F_RetPD:   return "F$RetPD";
    case SystemCallType::F_SSvc:    return "F$SSvc";
    case SystemCallType::F_IODel:   return "F$IODel";
    case SystemCallType::F_GProcP:  return "F$GProcP";
    case SystemCallType::F_Move:    return "F$Move";
    case SystemCallType::F_AllRAM:  return "F$AllRAM";
    case SystemCallType::F_Permit:  return "F$Permit";
    case SystemCallType::F_Protect: return "F$Protect";
    case SystemCallType::F_AllTsk:  return "F$AllTsk";
    case SystemCallType::F_DelTsk:  return "F$DelTsk";
    case SystemCallType::F_AllPrc:  return "F$AllPrc";
    case SystemCallType::F_DelPrc:  return "F$DelPrc";
    case SystemCallType::F_FModul:  return "F$FModul";
    case SystemCallType::F_SysDbg:  return "F$SysDbg";
    case SystemCallType::F_Event:   return "F$Event";
    case SystemCallType::F_Gregor:  return "F$Gregor";
    case SystemCallType::F_SysID:   return "F$SysID";
    case SystemCallType::F_Alarm:   return "F$Alarm";
    case SystemCallType::F_SigMask: return "F$SigMask";
    case SystemCallType::F_ChkMem:  return "F$ChkMem";
    case SystemCallType::F_UAcct:   return "F$UAcct";
    case SystemCallType::F_CCtl:    return "F$CCtl";
    case SystemCallType::F_GSPUMp:  return "F$GSPUMp";
    case SystemCallType::F_SRqCMem: return "F$SRqCMem";
    case SystemCallType::F_POSK:    return "F$POSK";
    case SystemCallType::F_Panic:   return "F$Panic";
    case SystemCallType::F_MBuf:    return "F$MBuf";
    case SystemCallType::F_Trans:   return "F$Trans";
    case SystemCallType::I_Attach:  return "I$Attach";
    case SystemCallType::I_Detach:  return "I$Detach";
    case SystemCallType::I_Dup:     return "I$Dup";
    case SystemCallType::I_Create:  return "I$Create";
    case SystemCallType::I_Open:    return "I$Open";
    case SystemCallType::I_MakDir:  return "I$MakDir";
    case SystemCallType::I_ChgDir:  return "I$ChgDir";
    case SystemCallType::I_Delete:  return "I$Delete";
    case SystemCallType::I_Seek:    return "I$Seek";
    case SystemCallType::I_Read:    return "I$Read";
    case SystemCallType::I_Write:   return "I$Write";
    case SystemCallType::I_ReadLn:  return "I$ReadLn";
    case SystemCallType::I_WritLn:  return "I$WritLn";
    case SystemCallType::I_GetStt:  return "I$GetStt";
    case SystemCallType::I_SetStt:  return "I$SetStt";
    case SystemCallType::I_Close:   return "I$Close";
    default: return "Unknown system call " + std::to_string((int)call);
    }
}

std::string systemCallInputsToString(const SystemCallType call, const std::map<SCC68070::Register, uint32_t>& regs, const std::function<const uint8_t*(const uint32_t)>& get)
{
    char args[256] = {0};
    switch(call)
    {
    case SystemCallType::F_Link:    snprintf(args, 256, "(a0).s=\"%s\" d0.w=%hd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::F_Load:    snprintf(args, 256, "(a0).s=\"%s\" d0.b=%hhd d1.l=%d", GETSTR(A0), REG(D0), REG(D1)); break;
    case SystemCallType::F_UnLink:  snprintf(args, 256, "a2=0x%X", REG(A2)); break;
    case SystemCallType::F_Fork:    snprintf(args, 256, "(a0).s=\"%s\" a1=0x%X d0.w=%hd d1.l=%d d2.l=%d d3.w=%hd d4.w=%hd", GETSTR(A0), REG(A1), REG(D0), REG(D1), REG(D2), REG(D3), REG(D4)); break;
    case SystemCallType::F_Wait:    return "";
    case SystemCallType::F_Chain:   snprintf(args, 256, "(a0).s=\"%s\" a1=0x%X d0.w=%hd d1.l=%d d2.l=%d d3.w=%hd d4.w=%hd", GETSTR(A0), REG(A1), REG(D0), REG(D1), REG(D2), REG(D3), REG(D4)); break;
    case SystemCallType::F_Exit:    snprintf(args, 256, "d1.w=%hd", REG(D1)); break;
    case SystemCallType::F_Mem:     snprintf(args, 256, "d0.l=%d", REG(D0)); break;
    case SystemCallType::F_Send:    snprintf(args, 256, "d0.w=%hd d1.w=%hd", REG(D0), REG(D1)); break;
    case SystemCallType::F_Icpt:    snprintf(args, 256, "a0=0x%X a6=0x%X", REG(A0), REG(A6)); break;
    case SystemCallType::F_Sleep:   snprintf(args, 256, "d0.l=%d", REG(D0)); break;
    case SystemCallType::F_SSpd:    snprintf(args, 256, "d0.w=%hd", REG(D0)); break;
    case SystemCallType::F_ID:      return "";
    case SystemCallType::F_SPrior:  snprintf(args, 256, "d0.w=%hd d1.w=%hd", REG(D0), REG(D1)); break;
    case SystemCallType::F_STrap:   snprintf(args, 256, "a0=0x%X a1=0x%X", REG(A0), REG(A1)); break;
    case SystemCallType::F_PErr:    snprintf(args, 256, "d0.w=%hd d1.w=%hd", REG(D0), REG(D1)); break;
    case SystemCallType::F_PrsNam:  snprintf(args, 256, "(a0).s=\"%s\"", GETSTR(A0)); break;
    case SystemCallType::F_CmpNam:  snprintf(args, 256, "(a0).s=\"%s\" (a1).s=\"%s\" d1.w=%hd }", GETSTR(A0), GETSTR(A1), REG(D1)); break;
    case SystemCallType::F_SchBit:  snprintf(args, 256, "a0=0x%X a1=0x%X d0.w=%hd d1.w=%hd", REG(A0), REG(A1), REG(D0), REG(D1)); break;
    case SystemCallType::F_AllBit:  snprintf(args, 256, "a0=0x%X d0.w=%hd d1.w=%hd", REG(A0), REG(D0), REG(D1)); break;
    case SystemCallType::F_DelBit:  snprintf(args, 256, "a0=0x%X d0.w=%hd d1.w=%hd", REG(A0), REG(D0), REG(D1)); break;
    case SystemCallType::F_Time:    snprintf(args, 256, "d0.w=%hd", REG(D0)); break;
    case SystemCallType::F_STime:   snprintf(args, 256, "d0.l=0x%X d1.l=0x%X", REG(D0), REG(D1)); break;
    case SystemCallType::F_CRC:     snprintf(args, 256, "a0=0x%X d0.l=%d d1.l=0x%X", REG(A0), REG(D0), REG(D1)); break;
    case SystemCallType::F_GPrDsc:  snprintf(args, 256, "a0=0x%X d0.w=%hd d1.w=%hd", REG(A0), REG(D0), REG(D1)); break;
    case SystemCallType::F_GBlkMp:  snprintf(args, 256, "a0=0x%X d0.l=%d d1.l=%d", REG(A0), REG(D0), REG(D1)); break;
    case SystemCallType::F_GModDr:  snprintf(args, 256, "a0=0x%X d1.l=%d", REG(A0), REG(D1)); break;
    case SystemCallType::F_CpyMem:  snprintf(args, 256, "a0=0x%X a1=0x%X d0.w=%hd d1.l=%d", REG(A0), REG(A1), REG(D0), REG(D1)); break;
    case SystemCallType::F_SUser:   snprintf(args, 256, "d1.l=%d", REG(D1)); break;
    case SystemCallType::F_UnLoad:  snprintf(args, 256, "(a0).s=\"%s\" d0.w=%hd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::F_RTE:     return "";
    case SystemCallType::F_GPrDBT:  snprintf(args, 256, "a0=0x%X d1.l=%d", REG(A0), REG(D1)); break;
    case SystemCallType::F_Julian:  snprintf(args, 256, "d0.l=0x%X d1.l=0x%X", REG(D0), REG(D1)); break;
    case SystemCallType::F_TLink:   snprintf(args, 256, "(a0).s=\"%s\" d0.w=%hd d1.l=%d", GETSTR(A0), REG(D0), REG(D1)); break;
    case SystemCallType::F_DFork:   snprintf(args, 256, "(a0).s=\"%s\" a1=0x%X a2=0x%X d0.w=%hd d1.l=%d d2.l=%d d3.w=%hd d4.w=%hd", GETSTR(A0), REG(A1), REG(A2), REG(D0), REG(D1), REG(D2), REG(D3), REG(D4)); break;
    case SystemCallType::F_DExec:   snprintf(args, 256, "d0.w=%hd d1.l=%d d2.w=%hd a0=0x%X", REG(D0), REG(D1), REG(D2), REG(A0)); break;
    case SystemCallType::F_DExit:   snprintf(args, 256, "d0.w=%hd", REG(D0)); break;
    case SystemCallType::F_DatMod:  snprintf(args, 256, "(a0).s=\"%s\" d0.l=%d d1.w=%hd d2.w=%hd d3.w=%hd d4.l=%d", GETSTR(A0), REG(D0), REG(D1), REG(D2), REG(D3), REG(D4)); break;
    case SystemCallType::F_SetCRC:  snprintf(args, 256, "a0=0x%X", REG(A0)); break;
    case SystemCallType::F_SetSys:  snprintf(args, 256, "d0.w=%hd d1.l=%d d2.l=%d", REG(D0), REG(D1), REG(D2)); break;
    case SystemCallType::F_SRqMem:  snprintf(args, 256, "d0.l=%d", REG(D0)); break;
    case SystemCallType::F_SRtMem:  snprintf(args, 256, "a2=0x%X d0.l=%d", REG(A2), REG(D0)); break;
    case SystemCallType::F_IRQ:     snprintf(args, 256, "a0=0x%X a2=0x%X a3:0x%X d0.b=%hhd d1.b=%hhd", REG(A0), REG(A2), REG(A3), REG(D0), REG(D1)); break;
    case SystemCallType::F_IOQu:    snprintf(args, 256, "d0.w=%hd", REG(D0)); break;
    case SystemCallType::F_AProc:   snprintf(args, 256, "a0=0x%X", REG(A0)); break;
    case SystemCallType::F_NProc:   return "";
    case SystemCallType::F_VModul:  snprintf(args, 256, "a0=0x%X d0.l=%d d1.l=%d", REG(A0), REG(D0), REG(D1)); break;
    case SystemCallType::F_FindPD:  snprintf(args, 256, "a0=0x%X d0.w=%hd", REG(A0), REG(D0)); break;
    case SystemCallType::F_AllPD:   snprintf(args, 256, "a0=0x%X", REG(A0)); break;
    case SystemCallType::F_RetPD:   snprintf(args, 256, "a0=0x%X d0.w=%hd", REG(A0), REG(D0)); break;
    case SystemCallType::F_SSvc:    snprintf(args, 256, "a1=0x%X a3=0x%X", REG(A1), REG(A3)); break;
    case SystemCallType::F_IODel:   snprintf(args, 256, "a0=0x%X", REG(A0)); break;
    case SystemCallType::F_GProcP:  snprintf(args, 256, "d0.w"); break;
    case SystemCallType::F_Move:    snprintf(args, 256, "a0=0x%X a2=0x%X d2.l=%d", REG(A0), REG(A2), REG(D2)); break;
    case SystemCallType::F_AllRAM:  return "xxx";
    case SystemCallType::F_Permit:  return "";
    case SystemCallType::F_Protect: return "";
    case SystemCallType::F_AllTsk:  return "";
    case SystemCallType::F_DelTsk:  return "";
    case SystemCallType::F_AllPrc:  return "";
    case SystemCallType::F_DelPrc:  snprintf(args, 256, "d0.w=%hd", REG(D0)); break;
    case SystemCallType::F_FModul:  return "xxx";
    case SystemCallType::F_SysDbg:  return "";
    case SystemCallType::F_Event:   snprintf(args, 256, "d1.w=%hd (%s) %s", REG(D1), eventNameToString(Event(REG(D1))).c_str(), eventInputsToString(Event(REG(D1)), regs, get).c_str()); break;
    case SystemCallType::F_Gregor:  snprintf(args, 256, "d0.l=%d d1.l=%d", REG(D0), REG(D1)); break;
    case SystemCallType::F_SysID:   snprintf(args, 256, "a0=0x%X a1=0x%X a2=0x%X a3=0x%X d0.l=%d", REG(A0), REG(A1), REG(A2), REG(A3), REG(D0)); break;
    case SystemCallType::F_Alarm:   snprintf(args, 256, "a0=0x%X d0.l=%d d1.w=%hd d2.l=%d d3.l=%d d4.l=%d", REG(A0), REG(D0), REG(D1), REG(D2), REG(D3), REG(D4)); break;
    case SystemCallType::F_SigMask: snprintf(args, 256, "d0.l=%d d1.l=%d", REG(D0), REG(D1)); break;
    case SystemCallType::F_ChkMem:  snprintf(args, 256, "a2=0x%X d0.l=%d d1.b=%hhd", REG(A2), REG(D0), REG(D1)); break;
    case SystemCallType::F_UAcct:   snprintf(args, 256, "d0.w=%hd a0=0x%X", REG(D0), REG(A0)); break;
    case SystemCallType::F_CCtl:    snprintf(args, 256, "d0.l=%d", REG(D0)); break;
    case SystemCallType::F_GSPUMp:  snprintf(args, 256, "a0=0x%X d0.w=%hd d2.l=%d", REG(A0), REG(D0), REG(D2)); break;
    case SystemCallType::F_SRqCMem: snprintf(args, 256, "d0.l=%d d1.l=%d", REG(D0), REG(D1)); break;
    case SystemCallType::F_POSK:    return "xxx";
    case SystemCallType::F_Panic:   snprintf(args, 256, "d0.l=%d", REG(D0)); break;
    case SystemCallType::F_MBuf:    return "xxx";
    case SystemCallType::F_Trans:   snprintf(args, 256, "a0=0x%X d0.l=%d d1.l=%d", REG(A0), REG(D0), REG(D1)); break;
    case SystemCallType::I_Attach:  snprintf(args, 256, "(a0).s=\"%s\" d0.b=%hhd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::I_Detach:  snprintf(args, 256, "a2=0x%X", REG(A2)); break;
    case SystemCallType::I_Dup:     snprintf(args, 256, "d0.w=%hd", REG(D0)); break;
    case SystemCallType::I_Create:  snprintf(args, 256, "(a0).s=\"%s\" d0.b=%hhd d1.w=%hd d2.l=%d", GETSTR(A0), REG(D0), REG(D1), REG(D2)); break;
    case SystemCallType::I_Open:    snprintf(args, 256, "(a0).s=\"%s\" d0.b=%hhd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::I_MakDir:  snprintf(args, 256, "(a0).s=\"%s\" d0.b=%hhd d1.w=%hd d2.l=%d", GETSTR(A0), REG(D0), REG(D1), REG(D2)); break;
    case SystemCallType::I_ChgDir:  snprintf(args, 256, "(a0).s=\"%s\" d0.b=%hhd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::I_Delete:  snprintf(args, 256, "(a0).s=\"%s\" d0.b=%hhd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::I_Seek:    snprintf(args, 256, "d0.w=%hd d1.l=%d", REG(D0), REG(D1)); break;
    case SystemCallType::I_Read:    snprintf(args, 256, "d0.w=%hd d1.l=%d a0=0x%X", REG(D0), REG(D1), REG(A0)); break;
    case SystemCallType::I_Write:   snprintf(args, 256, "d0.w=%hd d1.l=%d a0=0x%X", REG(D0), REG(D1), REG(A0)); break;
    case SystemCallType::I_ReadLn:  snprintf(args, 256, "d0.w=%hd d1.l=%d a0=0x%X", REG(D0), REG(D1), REG(A0)); break;
    case SystemCallType::I_WritLn:  snprintf(args, 256, "d0.w=%hd d1.l=%d a0=0x%X", REG(D0), REG(D1), REG(A0)); break;
    case SystemCallType::I_GetStt:  snprintf(args, 256, "d0.w=%hd d1.w=%hd (%s) d2.w=%hd d3.l=%d a0=0x%X", REG(D0), REG(D1), sttFunctionToString(REG(D1)).c_str(), REG(D2), REG(D3), REG(A0)); break;
    case SystemCallType::I_SetStt:  snprintf(args, 256, "d0.w=%hd d1.w=%hd (%s) d2.l=%d d3.w=%hd d4.l=%d a0=0x%X a1=0x%X", REG(D0), REG(D1), sttFunctionToString(REG(D1)).c_str(), REG(D2), REG(D3), REG(D4), REG(A0), REG(A1)); break;
    case SystemCallType::I_Close:   snprintf(args, 256, "d0.w=%hd", REG(D0)); break;
    default: snprintf(args, 256, "Unknown system call %d", (int)call);
    }
    return args;
}

std::string systemCallOutputsToString(const SystemCallType call, const std::map<SCC68070::Register, uint32_t>& regs, const std::function<const uint8_t*(const uint32_t)>& get)
{
    char args[256] = {0};
    switch(call)
    {
    case SystemCallType::F_Link:    snprintf(args, 256, "(a0).s=\"%s\" a1=0x%X a2=0x%X d0.w=%hd d1.w=%hd", GETSTR(A0), REG(A1), REG(A2), REG(D0), REG(D1)); break;
    case SystemCallType::F_Load:    snprintf(args, 256, "(a0).s=\"%s\" a1=0x%X a2=0x%X d0.w=%hd d1.w=%hd", GETSTR(A0), REG(A1), REG(A2), REG(D0), REG(D1)); break;
    case SystemCallType::F_UnLink:  return "";
    case SystemCallType::F_Fork:    snprintf(args, 256, "(a0).s=\"%s\" d0.w=%hd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::F_Wait:    snprintf(args, 256, "d0.w=%hd d1.w=%hd", REG(D0), REG(D1)); break;
    case SystemCallType::F_Chain:   return "";
    case SystemCallType::F_Exit:    return "";
    case SystemCallType::F_Mem:     snprintf(args, 256, "a1=0x%X d0.l=%d", REG(A1), REG(D0)); break;
    case SystemCallType::F_Send:    return "";
    case SystemCallType::F_Icpt:    return "";
    case SystemCallType::F_Sleep:   snprintf(args, 256, "d0.l=%d", REG(D0)); break;
    case SystemCallType::F_SSpd:    return "";
    case SystemCallType::F_ID:      snprintf(args, 256, "d0.w=%hd d1.l=%d d2.w=%hd", REG(D0), REG(D1), REG(D2)); break;
    case SystemCallType::F_SPrior:  return "";
    case SystemCallType::F_STrap:   return "";
    case SystemCallType::F_PErr:    return "";
    case SystemCallType::F_PrsNam:  snprintf(args, 256, "a0=\"%s\" a1=0x%X d0.b='%c' d1.w=%hd", GETSTR(A0), REG(A1), REG(D0), REG(D1)); break;
    case SystemCallType::F_CmpNam:  snprintf(args, 256, "carry=%d", REG(SR) & 1); break;
    case SystemCallType::F_SchBit:  snprintf(args, 256, "d0.w=%hd d1.w=%hd", REG(D0), REG(D1)); break;
    case SystemCallType::F_AllBit:  return "";
    case SystemCallType::F_DelBit:  return "";
    case SystemCallType::F_Time:    snprintf(args, 256, "d0.l=%d d1.l=%d d2.w=%hd d3.l=%d", REG(D0), REG(D1), REG(D2), REG(D3)); break;
    case SystemCallType::F_STime:   return "";
    case SystemCallType::F_CRC:     snprintf(args, 256, "d1.l=0x%X", REG(D1)); break;
    case SystemCallType::F_GPrDsc:  return "";
    case SystemCallType::F_GBlkMp:  snprintf(args, 256, "a0=0x%X d0.l=%d d1.l=%d d2.l=%d d3.l=%d", REG(A0), REG(D0), REG(D1), REG(D2), REG(D3)); break;
    case SystemCallType::F_GModDr:  snprintf(args, 256, "d1.l=%d", REG(D1)); break;
    case SystemCallType::F_CpyMem:  return "";
    case SystemCallType::F_SUser:   return "";
    case SystemCallType::F_UnLoad:  snprintf(args, 256, "(a0).s=\"%s\"", GETSTR(A0)); break;
    case SystemCallType::F_RTE:     return "";
    case SystemCallType::F_GPrDBT:  snprintf(args, 256, "d1.l=%d", REG(D1)); break;
    case SystemCallType::F_Julian:  snprintf(args, 256, "d0.l=%d d1.l=%d", REG(D0), REG(D1)); break;
    case SystemCallType::F_TLink:   snprintf(args, 256, "(a0).s=\"%s\" a1=0x%X a2=0x%X", GETSTR(A0), REG(A0), REG(A1)); break;
    case SystemCallType::F_DFork:   snprintf(args, 256, "(a0).s=\"%s\" a2=0x%X d0.w=%hd", GETSTR(A0), REG(A2), REG(D0)); break;
    case SystemCallType::F_DExec:   snprintf(args, 256, "d0.l=%d d1.l=%d d2.w=%hd d3.w=%hd d4.l=%d d5.w=%hd", REG(D0), REG(D1), REG(D2), REG(D3), REG(D4), REG(D5)); break;
    case SystemCallType::F_DExit:   return "";
    case SystemCallType::F_DatMod:  snprintf(args, 256, "(a0).s=\"%s\" a1=0x%X a2=0x%X d0.w=%hd d1.w=%hd", GETSTR(A0), REG(A1), REG(A2), REG(D0), REG(D1)); break;
    case SystemCallType::F_SetCRC:  return "";
    case SystemCallType::F_SetSys:  snprintf(args, 256, "d2.l=%d", REG(D2)); break;
    case SystemCallType::F_SRqMem:  snprintf(args, 256, "d0.l=%d a2=0x%X", REG(D0), REG(A2)); break;
    case SystemCallType::F_SRtMem:  return "";
    case SystemCallType::F_IRQ:     return "";
    case SystemCallType::F_IOQu:    return "";
    case SystemCallType::F_AProc:   return "";
    case SystemCallType::F_NProc:   return "";
    case SystemCallType::F_VModul:  snprintf(args, 256, "a2=0x%X", REG(A2)); break;
    case SystemCallType::F_FindPD:  snprintf(args, 256, "a1=0x%X", REG(A1)); break;
    case SystemCallType::F_AllPD:   snprintf(args, 256, "a1=0x%X d0.w=%hd", REG(A1), REG(D0)); break;
    case SystemCallType::F_RetPD:   return "";
    case SystemCallType::F_SSvc:    return "";
    case SystemCallType::F_IODel:   return "";
    case SystemCallType::F_GProcP:  snprintf(args, 256, "a1=0x%X", REG(A1)); break;
    case SystemCallType::F_Move:    return "";
    case SystemCallType::F_AllRAM:  return "xxx";
    case SystemCallType::F_Permit:  snprintf(args, 256, "a2=0x%X d0.l=%d d1.b=%hhd", REG(A2), REG(D0), REG(D1)); break;
    case SystemCallType::F_Protect: snprintf(args, 256, "a2=0x%X d0.l=%d d1.b=%hhd", REG(A2), REG(D0), REG(D1)); break;
    case SystemCallType::F_AllTsk:  return "";
    case SystemCallType::F_DelTsk:  return "";
    case SystemCallType::F_AllPrc:  snprintf(args, 256, "a2=0x%X", REG(A2)); break;
    case SystemCallType::F_DelPrc:  return "";
    case SystemCallType::F_FModul:  return "xxx";
    case SystemCallType::F_SysDbg:  return "";
    case SystemCallType::F_Event:   snprintf(args, 256, "%s", eventOutputsToString(Event(REG(D1)), regs, get).c_str()); break;
    case SystemCallType::F_Gregor:  snprintf(args, 256, "d0.l=0x%X d1.l=0x%X", REG(D0), REG(D1)); break;
    case SystemCallType::F_SysID:   snprintf(args, 256, "(a0).s=\"%s\" (a1).s=\"%s\" a2=0x%X a3=0x%X d0.l=%d d1.l=%d d2.l=%d d3.l=%d d4.l=%d d5.l=%d d6.l=%d d7.l=%d", GETSTR(A0), GETSTR(A1), REG(A2), REG(A3), REG(D0), REG(D1), REG(D2), REG(D3), REG(D4), REG(D5), REG(D6), REG(D7)); break;
    case SystemCallType::F_Alarm:   snprintf(args, 256, "d0.l=%d", REG(D0)); break;
    case SystemCallType::F_SigMask: return "";
    case SystemCallType::F_ChkMem:  return "";
    case SystemCallType::F_UAcct:   return "";
    case SystemCallType::F_CCtl:    return "";
    case SystemCallType::F_GSPUMp:  snprintf(args, 256, "a0=0x%X d2.l=%d", REG(A0), REG(D2)); break;
    case SystemCallType::F_SRqCMem: snprintf(args, 256, "a2=0x%X d0.l=%d", REG(A2), REG(D0)); break;
    case SystemCallType::F_POSK:    return "xxx";
    case SystemCallType::F_Panic:   return "";
    case SystemCallType::F_MBuf:    return "xxx";
    case SystemCallType::F_Trans:   snprintf(args, 256, "a0=0x%X d0.l=%d", REG(A0), REG(D0)); break;
    case SystemCallType::I_Attach:  snprintf(args, 256, "a2=0x%X", REG(A2)); break;
    case SystemCallType::I_Detach:  return "";
    case SystemCallType::I_Dup:     snprintf(args, 256, "d0.w=%hd", REG(D0)); break;
    case SystemCallType::I_Create:  snprintf(args, 256, "(a0).s=\"%s\" d0.w=%hd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::I_Open:    snprintf(args, 256, "(a0).s=\"%s\" d0.w=%hd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::I_MakDir:  snprintf(args, 256, "(a0).s=\"%s\"", GETSTR(A0)); break;
    case SystemCallType::I_ChgDir:  snprintf(args, 256, "(a0).s=\"%s\"", GETSTR(A0)); break;
    case SystemCallType::I_Delete:  snprintf(args, 256, "(a0).s=\"%s\"", GETSTR(A0)); break;
    case SystemCallType::I_Seek:    return "";
    case SystemCallType::I_Read:    snprintf(args, 256, "d1.l=%d", REG(D1)); break;
    case SystemCallType::I_Write:   snprintf(args, 256, "d1.l=%d", REG(D1)); break;
    case SystemCallType::I_ReadLn:  snprintf(args, 256, "d1.l=%d", REG(D1)); break;
    case SystemCallType::I_WritLn:  snprintf(args, 256, "d1.l=%d", REG(D1)); break;
    case SystemCallType::I_GetStt:  snprintf(args, 256, "d0.l=%d d1.l=%d d2.l=%d", REG(D0), REG(D1), REG(D2)); break;
    case SystemCallType::I_SetStt:  return "";
    case SystemCallType::I_Close:   return "";
    default: snprintf(args, 256, "Unknown system call %d", (int)call);
    }
    return args;
}

std::string errorNameToString(const Error error)
{
    switch(error)
    {
    case Error::E_1: return "E$1";
    case Error::E_2: return "E$2";
    case Error::E_3: return "E$3";
    case Error::E_4: return "E$4";

    case Error::E_IllFnc: return "E$IllFnc";
    case Error::E_FmtErr: return "E$FmtErr";
    case Error::E_NotNum: return "E$NotNum";
    case Error::E_IllArg: return "E$IllArg";

    case Error::E_BusErr: return "E$BusErr";
    case Error::E_AdrErr: return "E$AdrErr";
    case Error::E_IllIns: return "E$IllIns";
    case Error::E_ZerDiv: return "E$ZerDiv";
    case Error::E_Chk:    return "E$Chk";
    case Error::E_TrapV:  return "E$TrapV";
    case Error::E_Violat: return "E$Violat";
    case Error::E_Trace:  return "E$Trace";
    case Error::E_1010:   return "E$1010";
    case Error::E_1111:   return "E$1111";

    case Error::E_113: return "E$113";
    case Error::E_114: return "E$114";
    case Error::E_115: return "E$115";
    case Error::E_124: return "E$124";

    case Error::E_Trap1:  return "E$Trap1";
    case Error::E_Trap2:  return "E$Trap2";
    case Error::E_Trap3:  return "E$Trap3";
    case Error::E_Trap4:  return "E$Trap4";
    case Error::E_Trap5:  return "E$Trap5";
    case Error::E_Trap6:  return "E$Trap6";
    case Error::E_Trap7:  return "E$Trap7";
    case Error::E_Trap8:  return "E$Trap8";
    case Error::E_Trap9:  return "E$Trap9";
    case Error::E_Trap10: return "E$Trap10";
    case Error::E_Trap11: return "E$Trap11";
    case Error::E_Trap12: return "E$Trap12";
    case Error::E_Trap13: return "E$Trap13";
    case Error::E_Trap14: return "E$Trap14";
    case Error::E_Trap15: return "E$Trap15";
    case Error::E_FPUnordC: return "E$FPUnordC";
    case Error::E_FPInxact: return "E$FPInxact";
    case Error::E_FPDivZer: return "E$FPDivZer";
    case Error::E_FPUndrFl: return "E$FPUndrFl";
    case Error::E_FPOprErr: return "E$FPOprErr";
    case Error::E_FPOverFl: return "E$FPOverFl";
    case Error::E_FPNotNum: return "E$FPNotNum";
    case Error::E_155: return "E$155";
    case Error::E_156: return "E$156";
    case Error::E_157: return "E$157";
    case Error::E_158: return "E$158";

    case Error::E_Permit:   return "E$Permit";
    case Error::E_Differ:   return "E$Differ";
    case Error::E_StkOvf:   return "E$StkOvf";
    case Error::E_EvntID:   return "E$EvntID";
    case Error::E_EvNF:     return "E$EvNF";
    case Error::E_EvBusy:   return "E$EvBusy";
    case Error::E_EvParm:   return "E$EvParm";
    case Error::E_Damage:   return "E$Damage";
    case Error::E_BadRev:   return "E$BadRev";
    case Error::E_PthLost:  return "E$PthLost";
    case Error::E_BadPart:  return "E$BadPart";
    case Error::E_Hardware: return "E$Hardware";
    case Error::E_SectSize: return "E$SectSize";

    case Error::E_PthFul:  return "E$PthFul";
    case Error::E_BPNum:   return "E$BPNum";
    case Error::E_Poll:    return "E$Poll";
    case Error::E_BMode:   return "E$BMode";
    case Error::E_DevOvf:  return "E$DevOvf";
    case Error::E_BMID:    return "E$BMID";
    case Error::E_DirFul:  return "E$DirFul";
    case Error::E_MemFul:  return "E$MemFul";
    case Error::E_UnkSvc:  return "E$UnkSvc";
    case Error::E_ModBsy:  return "E$ModBsy";
    case Error::E_BPAddr:  return "E$BPAddr";
    case Error::E_EOF:     return "E$EOF";
    case Error::E_VctBsy:  return "E$VctBsy";
    case Error::E_NES:     return "E$NES";
    case Error::E_FNA:     return "E$FNA";
    case Error::E_BPNam:   return "E$BPNam";
    case Error::E_PNNF:    return "E$PNNF";
    case Error::E_SLF:     return "E$SLF";
    case Error::E_CEF:     return "E$CEF";
    case Error::E_IBA:     return "E$IBA";
    case Error::E_HangUp:  return "E$HangUp";
    case Error::E_MNF:     return "E$MNF";
    case Error::E_NoClk:   return "E$NoClk";
    case Error::E_DelSP:   return "E$DelSP";
    case Error::E_IPrcID:  return "E$IPrcID";
    case Error::E_Param:   return "E$Param";
    case Error::E_NoChld:  return "E$NoChld";
    case Error::E_ITrap:   return "E$ITrap";
    case Error::E_PrcAbt:  return "E$PrcAbt";
    case Error::E_PrcFul:  return "E$PrcFul";
    case Error::E_IForkP:  return "E$IForkP";
    case Error::E_KwnMod:  return "E$KwnMod";
    case Error::E_BMCRC:   return "E$BMCRC";
    case Error::E_USigP:   return "E$USigP";
    case Error::E_NEMod:   return "E$NEMod";
    case Error::E_BNam:    return "E$BNam";
    case Error::E_BMHP:    return "E$BMHP";
    case Error::E_NoRAM:   return "E$NoRAM";
    case Error::E_DNE:     return "E$DNE";
    case Error::E_NoTask:  return "E$NoTask";
    case Error::E_Unit:    return "E$Unit";
    case Error::E_Sect:    return "E$Sect";
    case Error::E_WP:      return "E$WP";
    case Error::E_CRC:     return "E$CRC";
    case Error::E_Read:    return "E$Read";
    case Error::E_Write:   return "E$Write";
    case Error::E_NotRdy:  return "E$NotRdy";
    case Error::E_Seek:    return "E$Seek";
    case Error::E_Full:    return "E$Full";
    case Error::E_BTyp:    return "E$BTyp";
    case Error::E_DevBsy:  return "E$DevBsy";
    case Error::E_DIDC:    return "E$DIDC";
    case Error::E_Lock:    return "E$Lock";
    case Error::E_Share:   return "E$Share";
    case Error::E_DeadLk:  return "E$DeadLk";
    case Error::E_Format:  return "E$Format";
    default: return "Unknown error " + std::to_string((int)error);
    }
}

} // namespace OS9
