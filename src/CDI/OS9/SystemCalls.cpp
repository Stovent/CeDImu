#include "SystemCalls.hpp"
#include "Stt.hpp"

#include <cstdio>

namespace OS9
{

#define GETSTR(areg) (get(regs.at(CPURegister::areg)) != nullptr ? (char*)get(regs.at(CPURegister::areg)) : "")
#define REG(reg) regs.at(CPURegister::reg)

std::string eventNameToString(const Event evt)
{
    switch(evt)
    {
    case Event::Ev$Link:  return "Ev$Link";
    case Event::Ev$UnLnk: return "Ev$UnLnk";
    case Event::Ev$Creat: return "Ev$Creat";
    case Event::Ev$Delet: return "Ev$Delet";
    case Event::Ev$Wait:  return "Ev$Wait";
    case Event::Ev$WaitR: return "Ev$WaitR";
    case Event::Ev$Read:  return "Ev$Read";
    case Event::Ev$Info:  return "Ev$Info";
    case Event::Ev$Signl: return "Ev$Signl";
    case Event::Ev$Signl_Ev$All: return "Ev$Signl+Ev$All";
    case Event::Ev$Pulse: return "Ev$Pulse";
    case Event::Ev$Pulse_Ev$All: return "Ev$Pulse+Ev$All";
    case Event::Ev$Set:   return "Ev$Set";
    case Event::Ev$Set_Ev$All: return "Ev$Set+Ev$All";
    case Event::Ev$SetR:  return "Ev$SetR";
    case Event::Ev$SetR_Ev$All: return "Ev$SetR+Ev$All";
    default: return "Unknown event " + std::to_string((int)evt);
    }
}

std::string eventInputsToString(const Event evt, const std::map<CPURegister, uint32_t>& regs, const std::function<const uint8_t*(const uint32_t)>& get)
{
    char event[256]= {0};
    switch(evt)
    {
    case Event::Ev$Link:  snprintf(event, 256, "(a0).s=\"%s\"", GETSTR(A0)); break;
    case Event::Ev$UnLnk: snprintf(event, 256, "d0.l=%d", REG(D0)); break;
    case Event::Ev$Creat: snprintf(event, 256, "(a0).s=\"%s\" d0.l=%d d2.w=%hd d3.w=%hd", GETSTR(A0), REG(D0), REG(D2), REG(D3)); break;
    case Event::Ev$Delet: snprintf(event, 256, "(a0).s=\"%s\"", GETSTR(A0)); break;
    case Event::Ev$Wait:  snprintf(event, 256, "d0.l=%d d2.l=%d d3.l=%d", REG(D0), REG(D2), REG(D3)); break;
    case Event::Ev$WaitR: snprintf(event, 256, "d0.l=%d d2.l=%d d3.l=%d", REG(D0), REG(D2), REG(D3)); break;
    case Event::Ev$Read:  snprintf(event, 256, "d0.l=%d", REG(D0)); break;
    case Event::Ev$Info:  snprintf(event, 256, "a0=0x%X d0.l=%d", REG(A0), REG(D0)); break;
    case Event::Ev$Signl: snprintf(event, 256, "d0.l=%d", REG(D0)); break;
    case Event::Ev$Signl_Ev$All: snprintf(event, 256, "d0.l=%d", REG(D0)); break;
    case Event::Ev$Pulse: snprintf(event, 256, "d0.l=%d d2.l=%d", REG(D0), REG(D2)); break;
    case Event::Ev$Pulse_Ev$All: snprintf(event, 256, "d0.l=%d d2.l=%d", REG(D0), REG(D2)); break;
    case Event::Ev$Set:   snprintf(event, 256, "d0.l=%d d2.l=%d", REG(D0), REG(D2)); break;
    case Event::Ev$Set_Ev$All: snprintf(event, 256, "d0.l=%d d2.l=%d", REG(D0), REG(D2)); break;
    case Event::Ev$SetR:  snprintf(event, 256, "d0.l=%d d2.l=%d", REG(D0), REG(D2)); break;
    case Event::Ev$SetR_Ev$All: snprintf(event, 256, "d0.l=%d d2.l=%d", REG(D0), REG(D2)); break;
    default: snprintf(event, 256, "Unknown event %d", (int)evt); break;
    }
    return event;
}

std::string eventOutputsToString(const Event evt, const std::map<CPURegister, uint32_t>& regs, const std::function<const uint8_t*(const uint32_t)>& get)
{
    char event[256]= {0};
    switch(evt)
    {
    case Event::Ev$Link:  snprintf(event, 256, "(a0).s=\"%s\" d0.l=%d", GETSTR(A0), REG(D0)); break;
    case Event::Ev$UnLnk: return "";
    case Event::Ev$Creat: snprintf(event, 256, "(a0).s=\"%s\" d0.l=%d", GETSTR(A0), REG(D0)); break;
    case Event::Ev$Delet: snprintf(event, 256, "(a0).s=\"%s\"", GETSTR(A0)); break;
    case Event::Ev$Wait:  snprintf(event, 256, "d1.l=%d", REG(D1)); break;
    case Event::Ev$WaitR: snprintf(event, 256, "d1.l=%d d2.l=%d d3.l=%d", REG(D1), REG(D2), REG(D3)); break;
    case Event::Ev$Read:  snprintf(event, 256, "d1.l=%d", REG(D1)); break;
    case Event::Ev$Info:  snprintf(event, 256, "d0.l=%d a0=%d", REG(D0), REG(A0)); break;
    case Event::Ev$Signl: return "";
    case Event::Ev$Signl_Ev$All: return "";
    case Event::Ev$Pulse: return "";
    case Event::Ev$Pulse_Ev$All: return "";
    case Event::Ev$Set:   snprintf(event, 256, "d1.l=%d", REG(D1)); break;
    case Event::Ev$Set_Ev$All: snprintf(event, 256, "d1.l=%d", REG(D1)); break;
    case Event::Ev$SetR:  snprintf(event, 256, "d1.l=%d", REG(D1)); break;
    case Event::Ev$SetR_Ev$All: snprintf(event, 256, "d1.l=%d", REG(D1)); break;
    default: snprintf(event, 256, "Unknown event %d", (int)evt); break;
    }
    return event;
}

std::string systemCallNameToString(const SystemCallType call)
{
    switch(call)
    {
    case SystemCallType::F$Link:    return "F$Link";
    case SystemCallType::F$Load:    return "F$Load";
    case SystemCallType::F$UnLink:  return "F$UnLink";
    case SystemCallType::F$Fork:    return "F$Fork";
    case SystemCallType::F$Wait:    return "F$Wait";
    case SystemCallType::F$Chain:   return "F$Chain";
    case SystemCallType::F$Exit:    return "F$Exit";
    case SystemCallType::F$Mem:     return "F$Mem";
    case SystemCallType::F$Send:    return "F$Send";
    case SystemCallType::F$Icpt:    return "F$Icpt";
    case SystemCallType::F$Sleep:   return "F$Sleep";
    case SystemCallType::F$SSpd:    return "F$SSpd";
    case SystemCallType::F$ID:      return "F$ID";
    case SystemCallType::F$SPrior:  return "F$SPrior";
    case SystemCallType::F$STrap:   return "F$STrap";
    case SystemCallType::F$PErr:    return "F$PErr";
    case SystemCallType::F$PrsNam:  return "F$PrsNam";
    case SystemCallType::F$CmpNam:  return "F$CmpNam";
    case SystemCallType::F$SchBit:  return "F$SchBit";
    case SystemCallType::F$AllBit:  return "F$AllBit";
    case SystemCallType::F$DelBit:  return "F$DelBit";
    case SystemCallType::F$Time:    return "F$Time";
    case SystemCallType::F$STime:   return "F$STime";
    case SystemCallType::F$CRC:     return "F$CRC";
    case SystemCallType::F$GPrDsc:  return "F$GPrDsc";
    case SystemCallType::F$GBlkMp:  return "F$GBlkMp";
    case SystemCallType::F$GModDr:  return "F$GModDr";
    case SystemCallType::F$CpyMem:  return "F$CpyMem";
    case SystemCallType::F$SUser:   return "F$SUser";
    case SystemCallType::F$UnLoad:  return "F$UnLoad";
    case SystemCallType::F$RTE:     return "F$RTE";
    case SystemCallType::F$GPrDBT:  return "F$GPrDBT";
    case SystemCallType::F$Julian:  return "F$Julian";
    case SystemCallType::F$TLink:   return "F$TLink";
    case SystemCallType::F$DFork:   return "F$DFork";
    case SystemCallType::F$DExec:   return "F$DExec";
    case SystemCallType::F$DExit:   return "F$DExit";
    case SystemCallType::F$DatMod:  return "F$DatMod";
    case SystemCallType::F$SetCRC:  return "F$SetCRC";
    case SystemCallType::F$SetSys:  return "F$SetSys";
    case SystemCallType::F$SRqMem:  return "F$SRqMem";
    case SystemCallType::F$SRtMem:  return "F$SRtMem";
    case SystemCallType::F$IRQ:     return "F$IRQ";
    case SystemCallType::F$IOQu:    return "F$IOQu";
    case SystemCallType::F$AProc:   return "F$AProc";
    case SystemCallType::F$NProc:   return "F$NProc";
    case SystemCallType::F$VModul:  return "F$VModul";
    case SystemCallType::F$FindPD:  return "F$FindPD";
    case SystemCallType::F$AllPD:   return "F$AllPD";
    case SystemCallType::F$RetPD:   return "F$RetPD";
    case SystemCallType::F$SSvc:    return "F$SSvc";
    case SystemCallType::F$IODel:   return "F$IODel";
    case SystemCallType::F$GProcP:  return "F$GProcP";
    case SystemCallType::F$Move:    return "F$Move";
    case SystemCallType::F$AllRAM:  return "F$AllRAM";
    case SystemCallType::F$Permit:  return "F$Permit";
    case SystemCallType::F$Protect: return "F$Protect";
    case SystemCallType::F$AllTsk:  return "F$AllTsk";
    case SystemCallType::F$DelTsk:  return "F$DelTsk";
    case SystemCallType::F$AllPrc:  return "F$AllPrc";
    case SystemCallType::F$DelPrc:  return "F$DelPrc";
    case SystemCallType::F$FModul:  return "F$FModul";
    case SystemCallType::F$SysDbg:  return "F$SysDbg";
    case SystemCallType::F$Event:   return "F$Event";
    case SystemCallType::F$Gregor:  return "F$Gregor";
    case SystemCallType::F$SysID:   return "F$SysID";
    case SystemCallType::F$Alarm:   return "F$Alarm";
    case SystemCallType::F$SigMask: return "F$SigMask";
    case SystemCallType::F$ChkMem:  return "F$ChkMem";
    case SystemCallType::F$UAcct:   return "F$UAcct";
    case SystemCallType::F$CCtl:    return "F$CCtl";
    case SystemCallType::F$GSPUMp:  return "F$GSPUMp";
    case SystemCallType::F$SRqCMem: return "F$SRqCMem";
    case SystemCallType::F$POSK:    return "F$POSK";
    case SystemCallType::F$Panic:   return "F$Panic";
    case SystemCallType::F$MBuf:    return "F$MBuf";
    case SystemCallType::F$Trans:   return "F$Trans";
    case SystemCallType::I$Attach:  return "I$Attach";
    case SystemCallType::I$Detach:  return "I$Detach";
    case SystemCallType::I$Dup:     return "I$Dup";
    case SystemCallType::I$Create:  return "I$Create";
    case SystemCallType::I$Open:    return "I$Open";
    case SystemCallType::I$MakDir:  return "I$MakDir";
    case SystemCallType::I$ChgDir:  return "I$ChgDir";
    case SystemCallType::I$Delete:  return "I$Delete";
    case SystemCallType::I$Seek:    return "I$Seek";
    case SystemCallType::I$Read:    return "I$Read";
    case SystemCallType::I$Write:   return "I$Write";
    case SystemCallType::I$ReadLn:  return "I$ReadLn";
    case SystemCallType::I$WritLn:  return "I$WritLn";
    case SystemCallType::I$GetStt:  return "I$GetStt";
    case SystemCallType::I$SetStt:  return "I$SetStt";
    case SystemCallType::I$Close:   return "I$Close";
    default: return "Unknown system call " + std::to_string((int)call);
    }
}

std::string systemCallInputsToString(const SystemCallType call, const std::map<CPURegister, uint32_t>& regs, const std::function<const uint8_t*(const uint32_t)>& get)
{
    char args[256] = {0};
    switch(call)
    {
    case SystemCallType::F$Link:    snprintf(args, 256, "(a0).s=\"%s\" d0.w=%hd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::F$Load:    snprintf(args, 256, "(a0).s=\"%s\" d0.b=%hhd d1.l=%d", GETSTR(A0), REG(D0), REG(D1)); break;
    case SystemCallType::F$UnLink:  snprintf(args, 256, "a2=0x%X", REG(A2)); break;
    case SystemCallType::F$Fork:    snprintf(args, 256, "(a0).s=\"%s\" a1=0x%X d0.w=%hd d1.l=%d d2.l=%d d3.w=%hd d4.w=%hd", GETSTR(A0), REG(A1), REG(D0), REG(D1), REG(D2), REG(D3), REG(D4)); break;
    case SystemCallType::F$Wait:    return "";
    case SystemCallType::F$Chain:   snprintf(args, 256, "(a0).s=\"%s\" a1=0x%X d0.w=%hd d1.l=%d d2.l=%d d3.w=%hd d4.w=%hd", GETSTR(A0), REG(A1), REG(D0), REG(D1), REG(D2), REG(D3), REG(D4)); break;
    case SystemCallType::F$Exit:    snprintf(args, 256, "d1.w=%hd", REG(D1)); break;
    case SystemCallType::F$Mem:     snprintf(args, 256, "d0.l=%d", REG(D0)); break;
    case SystemCallType::F$Send:    snprintf(args, 256, "d0.w=%hd d1.w=%hd", REG(D0), REG(D1)); break;
    case SystemCallType::F$Icpt:    snprintf(args, 256, "a0=0x%X a6=0x%X", REG(A0), REG(A6)); break;
    case SystemCallType::F$Sleep:   snprintf(args, 256, "d0.l=%d", REG(D0)); break;
    case SystemCallType::F$SSpd:    snprintf(args, 256, "d0.w=%hd", REG(D0)); break;
    case SystemCallType::F$ID:      return "";
    case SystemCallType::F$SPrior:  snprintf(args, 256, "d0.w=%hd d1.w=%hd", REG(D0), REG(D1)); break;
    case SystemCallType::F$STrap:   snprintf(args, 256, "a0=0x%X a1=0x%X", REG(A0), REG(A1)); break;
    case SystemCallType::F$PErr:    snprintf(args, 256, "d0.w=%hd d1.w=%hd", REG(D0), REG(D1)); break;
    case SystemCallType::F$PrsNam:  snprintf(args, 256, "(a0).s=\"%s\"", GETSTR(A0)); break;
    case SystemCallType::F$CmpNam:  snprintf(args, 256, "(a0).s=\"%s\" (a1).s=\"%s\" d1.w=%hd }", GETSTR(A0), GETSTR(A1), REG(D1)); break;
    case SystemCallType::F$SchBit:  snprintf(args, 256, "a0=0x%X a1=0x%X d0.w=%hd d1.w=%hd", REG(A0), REG(A1), REG(D0), REG(D1)); break;
    case SystemCallType::F$AllBit:  snprintf(args, 256, "a0=0x%X d0.w=%hd d1.w=%hd", REG(A0), REG(D0), REG(D1)); break;
    case SystemCallType::F$DelBit:  snprintf(args, 256, "a0=0x%X d0.w=%hd d1.w=%hd", REG(A0), REG(D0), REG(D1)); break;
    case SystemCallType::F$Time:    snprintf(args, 256, "d0.w=%hd", REG(D0)); break;
    case SystemCallType::F$STime:   snprintf(args, 256, "d0.l=0x%X d1.l=0x%X", REG(D0), REG(D1)); break;
    case SystemCallType::F$CRC:     snprintf(args, 256, "a0=0x%X d0.l=%d d1.l=0x%X", REG(A0), REG(D0), REG(D1)); break;
    case SystemCallType::F$GPrDsc:  snprintf(args, 256, "a0=0x%X d0.w=%hd d1.w=%hd", REG(A0), REG(D0), REG(D1)); break;
    case SystemCallType::F$GBlkMp:  snprintf(args, 256, "a0=0x%X d0.l=%d d1.l=%d", REG(A0), REG(D0), REG(D1)); break;
    case SystemCallType::F$GModDr:  snprintf(args, 256, "a0=0x%X d1.l=%d", REG(A0), REG(D1)); break;
    case SystemCallType::F$CpyMem:  snprintf(args, 256, "a0=0x%X a1=0x%X d0.w=%hd d1.l=%d", REG(A0), REG(A1), REG(D0), REG(D1)); break;
    case SystemCallType::F$SUser:   snprintf(args, 256, "d1.l=%d", REG(D1)); break;
    case SystemCallType::F$UnLoad:  snprintf(args, 256, "(a0).s=\"%s\" d0.w=%hd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::F$RTE:     return "";
    case SystemCallType::F$GPrDBT:  snprintf(args, 256, "a0=0x%X d1.l=%d", REG(A0), REG(D1)); break;
    case SystemCallType::F$Julian:  snprintf(args, 256, "d0.l=0x%X d1.l=0x%X", REG(D0), REG(D1)); break;
    case SystemCallType::F$TLink:   snprintf(args, 256, "(a0).s=\"%s\" d0.w=%hd d1.l=%d", GETSTR(A0), REG(D0), REG(D1)); break;
    case SystemCallType::F$DFork:   snprintf(args, 256, "(a0).s=\"%s\" a1=0x%X a2=0x%X d0.w=%hd d1.l=%d d2.l=%d d3.w=%hd d4.w=%hd", GETSTR(A0), REG(A1), REG(A2), REG(D0), REG(D1), REG(D2), REG(D3), REG(D4)); break;
    case SystemCallType::F$DExec:   snprintf(args, 256, "d0.w=%hd d1.l=%d d2.w=%hd a0=0x%X", REG(D0), REG(D1), REG(D2), REG(A0)); break;
    case SystemCallType::F$DExit:   snprintf(args, 256, "d0.w=%hd", REG(D0)); break;
    case SystemCallType::F$DatMod:  snprintf(args, 256, "(a0).s=\"%s\" d0.l=%d d1.w=%hd d2.w=%hd d3.w=%hd d4.l=%d", GETSTR(A0), REG(D0), REG(D1), REG(D2), REG(D3), REG(D4)); break;
    case SystemCallType::F$SetCRC:  snprintf(args, 256, "a0=0x%X", REG(A0)); break;
    case SystemCallType::F$SetSys:  snprintf(args, 256, "d0.w=%hd d1.l=%d d2.l=%d", REG(D0), REG(D1), REG(D2)); break;
    case SystemCallType::F$SRqMem:  snprintf(args, 256, "d0.l=%d", REG(D0)); break;
    case SystemCallType::F$SRtMem:  snprintf(args, 256, "a2=0x%X d0.l=%d", REG(A2), REG(D0)); break;
    case SystemCallType::F$IRQ:     snprintf(args, 256, "a0=0x%X a2=0x%X a3:0x%X d0.b=%hhd d1.b=%hhd", REG(A0), REG(A2), REG(A3), REG(D0), REG(D1)); break;
    case SystemCallType::F$IOQu:    snprintf(args, 256, "d0.w=%hd", REG(D0)); break;
    case SystemCallType::F$AProc:   snprintf(args, 256, "a0=0x%X", REG(A0)); break;
    case SystemCallType::F$NProc:   return "";
    case SystemCallType::F$VModul:  snprintf(args, 256, "a0=0x%X d0.l=%d d1.l=%d", REG(A0), REG(D0), REG(D1)); break;
    case SystemCallType::F$FindPD:  snprintf(args, 256, "a0=0x%X d0.w=%hd", REG(A0), REG(D0)); break;
    case SystemCallType::F$AllPD:   snprintf(args, 256, "a0=0x%X", REG(A0)); break;
    case SystemCallType::F$RetPD:   snprintf(args, 256, "a0=0x%X d0.w=%hd", REG(A0), REG(D0)); break;
    case SystemCallType::F$SSvc:    snprintf(args, 256, "a1=0x%X a3=0x%X", REG(A1), REG(A3)); break;
    case SystemCallType::F$IODel:   snprintf(args, 256, "a0=0x%X", REG(A0)); break;
    case SystemCallType::F$GProcP:  snprintf(args, 256, "d0.w"); break;
    case SystemCallType::F$Move:    snprintf(args, 256, "a0=0x%X a2=0x%X d2.l=%d", REG(A0), REG(A2), REG(D2)); break;
    case SystemCallType::F$AllRAM:  return "xxx";
    case SystemCallType::F$Permit:  return "";
    case SystemCallType::F$Protect: return "";
    case SystemCallType::F$AllTsk:  return "";
    case SystemCallType::F$DelTsk:  return "";
    case SystemCallType::F$AllPrc:  return "";
    case SystemCallType::F$DelPrc:  snprintf(args, 256, "d0.w=%hd", REG(D0)); break;
    case SystemCallType::F$FModul:  return "xxx";
    case SystemCallType::F$SysDbg:  return "";
    case SystemCallType::F$Event:   snprintf(args, 256, "d1.w=%hd (%s) %s", REG(D1), eventNameToString(Event(REG(D1))).c_str(), eventInputsToString(Event(REG(D1)), regs, get).c_str()); break;
    case SystemCallType::F$Gregor:  snprintf(args, 256, "d0.l=%d d1.l=%d", REG(D0), REG(D1)); break;
    case SystemCallType::F$SysID:   snprintf(args, 256, "a0=0x%X a1=0x%X a2=0x%X a3=0x%X d0.l=%d", REG(A0), REG(A1), REG(A2), REG(A3), REG(D0)); break;
    case SystemCallType::F$Alarm:   snprintf(args, 256, "a0=0x%X d0.l=%d d1.w=%hd d2.l=%d d3.l=%d d4.l=%d", REG(A0), REG(D0), REG(D1), REG(D2), REG(D3), REG(D4)); break;
    case SystemCallType::F$SigMask: snprintf(args, 256, "d0.l=%d d1.l=%d", REG(D0), REG(D1)); break;
    case SystemCallType::F$ChkMem:  snprintf(args, 256, "a2=0x%X d0.l=%d d1.b=%hhd", REG(A2), REG(D0), REG(D1)); break;
    case SystemCallType::F$UAcct:   snprintf(args, 256, "d0.w=%hd a0=0x%X", REG(D0), REG(A0)); break;
    case SystemCallType::F$CCtl:    snprintf(args, 256, "d0.l=%d", REG(D0)); break;
    case SystemCallType::F$GSPUMp:  snprintf(args, 256, "a0=0x%X d0.w=%hd d2.l=%d", REG(A0), REG(D0), REG(D2)); break;
    case SystemCallType::F$SRqCMem: snprintf(args, 256, "d0.l=%d d1.l=%d", REG(D0), REG(D1)); break;
    case SystemCallType::F$POSK:    return "xxx";
    case SystemCallType::F$Panic:   snprintf(args, 256, "d0.l=%d", REG(D0)); break;
    case SystemCallType::F$MBuf:    return "xxx";
    case SystemCallType::F$Trans:   snprintf(args, 256, "a0=0x%X d0.l=%d d1.l=%d", REG(A0), REG(D0), REG(D1)); break;
    case SystemCallType::I$Attach:  snprintf(args, 256, "(a0).s=\"%s\" d0.b=%hhd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::I$Detach:  snprintf(args, 256, "a2=0x%X", REG(A2)); break;
    case SystemCallType::I$Dup:     snprintf(args, 256, "d0.w=%hd", REG(D0)); break;
    case SystemCallType::I$Create:  snprintf(args, 256, "(a0).s=\"%s\" d0.b=%hhd d1.w=%hd d2.l=%d", GETSTR(A0), REG(D0), REG(D1), REG(D2)); break;
    case SystemCallType::I$Open:    snprintf(args, 256, "(a0).s=\"%s\" d0.b=%hhd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::I$MakDir:  snprintf(args, 256, "(a0).s=\"%s\" d0.b=%hhd d1.w=%hd d2.l=%d", GETSTR(A0), REG(D0), REG(D1), REG(D2)); break;
    case SystemCallType::I$ChgDir:  snprintf(args, 256, "(a0).s=\"%s\" d0.b=%hhd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::I$Delete:  snprintf(args, 256, "(a0).s=\"%s\" d0.b=%hhd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::I$Seek:    snprintf(args, 256, "d0.w=%hd d1.l=%d", REG(D0), REG(D1)); break;
    case SystemCallType::I$Read:    snprintf(args, 256, "d0.w=%hd d1.l=%d a0=0x%X", REG(D0), REG(D1), REG(A0)); break;
    case SystemCallType::I$Write:   snprintf(args, 256, "d0.w=%hd d1.l=%d a0=0x%X", REG(D0), REG(D1), REG(A0)); break;
    case SystemCallType::I$ReadLn:  snprintf(args, 256, "d0.w=%hd d1.l=%d a0=0x%X", REG(D0), REG(D1), REG(A0)); break;
    case SystemCallType::I$WritLn:  snprintf(args, 256, "d0.w=%hd d1.l=%d a0=0x%X", REG(D0), REG(D1), REG(A0)); break;
    case SystemCallType::I$GetStt:  snprintf(args, 256, "d0.w=%hd d1.w=%hd (%s) d2.w=%hd d3.l=%d a0=0x%X", REG(D0), REG(D1), sttFunctionToString(SttFunction(REG(D1))).c_str(), REG(D2), REG(D3), REG(A0)); break;
    case SystemCallType::I$SetStt:  snprintf(args, 256, "d0.w=%hd d1.w=%hd (%s) d2.l=%d d3.w=%hd d4.l=%d a0=0x%X a1=0x%X", REG(D0), REG(D1), sttFunctionToString(SttFunction(REG(D1))).c_str(), REG(D2), REG(D3), REG(D4), REG(A0), REG(A1)); break;
    case SystemCallType::I$Close:   snprintf(args, 256, "d0.w=%hd", REG(D0)); break;
    default: snprintf(args, 256, "Unknown system call %d", (int)call);
    }
    return args;
}

std::string systemCallOutputsToString(const SystemCallType call, const std::map<CPURegister, uint32_t>& regs, const std::function<const uint8_t*(const uint32_t)>& get)
{
    char args[256] = {0};
    switch(call)
    {
    case SystemCallType::F$Link:    snprintf(args, 256, "(a0).s=\"%s\" a1=0x%X a2=0x%X d0.w=%hd d1.w=%hd", GETSTR(A0), REG(A1), REG(A2), REG(D0), REG(D1)); break;
    case SystemCallType::F$Load:    snprintf(args, 256, "(a0).s=\"%s\" a1=0x%X a2=0x%X d0.w=%hd d1.w=%hd", GETSTR(A0), REG(A1), REG(A2), REG(D0), REG(D1)); break;
    case SystemCallType::F$UnLink:  return "";
    case SystemCallType::F$Fork:    snprintf(args, 256, "(a0).s=\"%s\" d0.w=%hd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::F$Wait:    snprintf(args, 256, "d0.w=%hd d1.w=%hd", REG(D0), REG(D1)); break;
    case SystemCallType::F$Chain:   return "";
    case SystemCallType::F$Exit:    return "";
    case SystemCallType::F$Mem:     snprintf(args, 256, "a1=0x%X d0.l=%d", REG(A1), REG(D0)); break;
    case SystemCallType::F$Send:    return "";
    case SystemCallType::F$Icpt:    return "";
    case SystemCallType::F$Sleep:   snprintf(args, 256, "d0.l=%d", REG(D0)); break;
    case SystemCallType::F$SSpd:    return "";
    case SystemCallType::F$ID:      snprintf(args, 256, "d0.w=%hd d1.l=%d d2.w=%hd", REG(D0), REG(D1), REG(D2)); break;
    case SystemCallType::F$SPrior:  return "";
    case SystemCallType::F$STrap:   return "";
    case SystemCallType::F$PErr:    return "";
    case SystemCallType::F$PrsNam:  snprintf(args, 256, "a0=\"%s\" a1=0x%X d0.b='%c' d1.w=%hd", GETSTR(A0), REG(A1), REG(D0), REG(D1)); break;
    case SystemCallType::F$CmpNam:  snprintf(args, 256, "carry=%d", REG(SR) & 1); break;
    case SystemCallType::F$SchBit:  snprintf(args, 256, "d0.w=%hd d1.w=%hd", REG(D0), REG(D1)); break;
    case SystemCallType::F$AllBit:  return "";
    case SystemCallType::F$DelBit:  return "";
    case SystemCallType::F$Time:    snprintf(args, 256, "d0.l=%d d1.l=%d d2.w=%hd d3.l=%d", REG(D0), REG(D1), REG(D2), REG(D3)); break;
    case SystemCallType::F$STime:   return "";
    case SystemCallType::F$CRC:     snprintf(args, 256, "d1.l=0x%X", REG(D1)); break;
    case SystemCallType::F$GPrDsc:  return "";
    case SystemCallType::F$GBlkMp:  snprintf(args, 256, "a0=0x%X d0.l=%d d1.l=%d d2.l=%d d3.l=%d", REG(A0), REG(D0), REG(D1), REG(D2), REG(D3)); break;
    case SystemCallType::F$GModDr:  snprintf(args, 256, "d1.l=%d", REG(D1)); break;
    case SystemCallType::F$CpyMem:  return "";
    case SystemCallType::F$SUser:   return "";
    case SystemCallType::F$UnLoad:  snprintf(args, 256, "(a0).s=\"%s\"", GETSTR(A0)); break;
    case SystemCallType::F$RTE:     return "";
    case SystemCallType::F$GPrDBT:  snprintf(args, 256, "d1.l=%d", REG(D1)); break;
    case SystemCallType::F$Julian:  snprintf(args, 256, "d0.l=%d d1.l=%d", REG(D0), REG(D1)); break;
    case SystemCallType::F$TLink:   snprintf(args, 256, "(a0).s=\"%s\" a1=0x%X a2=0x%X", GETSTR(A0), REG(A0), REG(A1)); break;
    case SystemCallType::F$DFork:   snprintf(args, 256, "(a0).s=\"%s\" a2=0x%X d0.w=%hd", GETSTR(A0), REG(A2), REG(D0)); break;
    case SystemCallType::F$DExec:   snprintf(args, 256, "d0.l=%d d1.l=%d d2.w=%hd d3.w=%hd d4.l=%d d5.w=%hd", REG(D0), REG(D1), REG(D2), REG(D3), REG(D4), REG(D5)); break;
    case SystemCallType::F$DExit:   return "";
    case SystemCallType::F$DatMod:  snprintf(args, 256, "(a0).s=\"%s\" a1=0x%X a2=0x%X d0.w=%hd d1.w=%hd", GETSTR(A0), REG(A1), REG(A2), REG(D0), REG(D1)); break;
    case SystemCallType::F$SetCRC:  return "";
    case SystemCallType::F$SetSys:  snprintf(args, 256, "d2.l=%d", REG(D2)); break;
    case SystemCallType::F$SRqMem:  snprintf(args, 256, "d0.l=%d a2=0x%X", REG(D0), REG(A2)); break;
    case SystemCallType::F$SRtMem:  return "";
    case SystemCallType::F$IRQ:     return "";
    case SystemCallType::F$IOQu:    return "";
    case SystemCallType::F$AProc:   return "";
    case SystemCallType::F$NProc:   return "";
    case SystemCallType::F$VModul:  snprintf(args, 256, "a2=0x%X", REG(A2)); break;
    case SystemCallType::F$FindPD:  snprintf(args, 256, "a1=0x%X", REG(A1)); break;
    case SystemCallType::F$AllPD:   snprintf(args, 256, "a1=0x%X d0.w=%hd", REG(A1), REG(D0)); break;
    case SystemCallType::F$RetPD:   return "";
    case SystemCallType::F$SSvc:    return "";
    case SystemCallType::F$IODel:   return "";
    case SystemCallType::F$GProcP:  snprintf(args, 256, "a1=0x%X", REG(A1)); break;
    case SystemCallType::F$Move:    return "";
    case SystemCallType::F$AllRAM:  return "xxx";
    case SystemCallType::F$Permit:  snprintf(args, 256, "a2=0x%X d0.l=%d d1.b=%hhd", REG(A2), REG(D0), REG(D1)); break;
    case SystemCallType::F$Protect: snprintf(args, 256, "a2=0x%X d0.l=%d d1.b=%hhd", REG(A2), REG(D0), REG(D1)); break;
    case SystemCallType::F$AllTsk:  return "";
    case SystemCallType::F$DelTsk:  return "";
    case SystemCallType::F$AllPrc:  snprintf(args, 256, "a2=0x%X", REG(A2)); break;
    case SystemCallType::F$DelPrc:  return "";
    case SystemCallType::F$FModul:  return "xxx";
    case SystemCallType::F$SysDbg:  return "";
    case SystemCallType::F$Event:   snprintf(args, 256, "%s", eventOutputsToString(Event(REG(D1)), regs, get).c_str()); break;
    case SystemCallType::F$Gregor:  snprintf(args, 256, "d0.l=0x%X d1.l=0x%X", REG(D0), REG(D1)); break;
    case SystemCallType::F$SysID:   snprintf(args, 256, "(a0).s=\"%s\" (a1).s=\"%s\" a2=0x%X a3=0x%X d0.l=%d d1.l=%d d2.l=%d d3.l=%d d4.l=%d d5.l=%d d6.l=%d d7.l=%d", GETSTR(A0), GETSTR(A1), REG(A2), REG(A3), REG(D0), REG(D1), REG(D2), REG(D3), REG(D4), REG(D5), REG(D6), REG(D7)); break;
    case SystemCallType::F$Alarm:   snprintf(args, 256, "d0.l=%d", REG(D0)); break;
    case SystemCallType::F$SigMask: return "";
    case SystemCallType::F$ChkMem:  return "";
    case SystemCallType::F$UAcct:   return "";
    case SystemCallType::F$CCtl:    return "";
    case SystemCallType::F$GSPUMp:  snprintf(args, 256, "a0=0x%X d2.l=%d", REG(A0), REG(D2)); break;
    case SystemCallType::F$SRqCMem: snprintf(args, 256, "a2=0x%X d0.l=%d", REG(A2), REG(D0)); break;
    case SystemCallType::F$POSK:    return "xxx";
    case SystemCallType::F$Panic:   return "";
    case SystemCallType::F$MBuf:    return "xxx";
    case SystemCallType::F$Trans:   snprintf(args, 256, "a0=0x%X d0.l=%d", REG(A0), REG(D0)); break;
    case SystemCallType::I$Attach:  snprintf(args, 256, "a2=0x%X", REG(A2)); break;
    case SystemCallType::I$Detach:  return "";
    case SystemCallType::I$Dup:     snprintf(args, 256, "d0.w=%hd", REG(D0)); break;
    case SystemCallType::I$Create:  snprintf(args, 256, "(a0).s=\"%s\" d0.w=%hd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::I$Open:    snprintf(args, 256, "(a0).s=\"%s\" d0.w=%hd", GETSTR(A0), REG(D0)); break;
    case SystemCallType::I$MakDir:  snprintf(args, 256, "(a0).s=\"%s\"", GETSTR(A0)); break;
    case SystemCallType::I$ChgDir:  snprintf(args, 256, "(a0).s=\"%s\"", GETSTR(A0)); break;
    case SystemCallType::I$Delete:  snprintf(args, 256, "(a0).s=\"%s\"", GETSTR(A0)); break;
    case SystemCallType::I$Seek:    return "";
    case SystemCallType::I$Read:    snprintf(args, 256, "d1.l=%d", REG(D1)); break;
    case SystemCallType::I$Write:   snprintf(args, 256, "d1.l=%d", REG(D1)); break;
    case SystemCallType::I$ReadLn:  snprintf(args, 256, "d1.l=%d", REG(D1)); break;
    case SystemCallType::I$WritLn:  snprintf(args, 256, "d1.l=%d", REG(D1)); break;
    case SystemCallType::I$GetStt:  snprintf(args, 256, "d0.l=%d d1.l=%d d2.l=%d", REG(D0), REG(D1), REG(D2)); break;
    case SystemCallType::I$SetStt:  return "";
    case SystemCallType::I$Close:   return "";
    default: snprintf(args, 256, "Unknown system call %d", (int)call);
    }
    return args;
}

} // namespace OS9
