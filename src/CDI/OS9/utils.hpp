#ifndef OS9_UTILS_HPP
#define OS9_UTILS_HPP

#include "../common/utils.hpp"

namespace OS9
{

inline std::string disassembleOS9Event(const uint16_t word, const uint32_t D[8], const uint32_t A[8], const std::function<const uint8_t*(const uint32_t)> get)
{
    char event[256] = {0};
    switch(word)
    {
    case 0x0000: snprintf(event, 256, "Ev$Link { (a0).s:\"%s\" } => { (a0).s d0.l }", get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""); break;
    case 0x0001: snprintf(event, 256, "Ev$UnLnk { d0.l:%d }", D[0]); break;
    case 0x0002: snprintf(event, 256, "Ev$Creat { (a0).s:\"%s\" d0.l:%d d2.w:%hd d3.w:%hd } => { (a0).s d0.l }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""), D[0], D[2], D[3]); break;
    case 0x0003: snprintf(event, 256, "Ev$Delet { (a0).s:\"%s\" } => { (a0).s }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)"")); break;
    case 0x0004: snprintf(event, 256, "Ev$Wait { d0.l:%d d2.l:%d d3.l:%d } => { d1.l }", D[0], D[2], D[3]); break;
    case 0x0005: snprintf(event, 256, "Ev$WaitR { d0.l:%d d2.l:%d d3.l:%d } => { d1.l d2.l d3.l }", D[0], D[2], D[3]); break;
    case 0x0006: snprintf(event, 256, "Ev$Read { d0.l:%d } => { d1.l }", D[0]); break;
    case 0x0007: snprintf(event, 256, "Ev$Info { a0:0x%X d0.l:%d } => { d0.l a0 }", A[0], D[0]); break;
    case 0x0008: snprintf(event, 256, "Ev$Signl { d0.l:%d }", D[0]); break;
    case 0x8008: snprintf(event, 256, "Ev$Signl+Ev$All { d0.l:%d }", D[0]); break;
    case 0x0009: snprintf(event, 256, "Ev$Pulse { d0.l:%d d2.l:%d }", D[0], D[2]); break;
    case 0x8009: snprintf(event, 256, "Ev$Pulse+Ev$All { d0.l:%d d2.l:%d }", D[0], D[2]); break;
    case 0x000A: snprintf(event, 256, "Ev$Set { d0.l:%d d2.l:%d } => { d1.l }", D[0], D[2]); break;
    case 0x800A: snprintf(event, 256, "Ev$Set+Ev$All { d0.l:%d d2.l:%d } => { d1.l }", D[0], D[2]); break;
    case 0x000B: snprintf(event, 256, "Ev$SetR { d0.l:%d d2.l:%d } => { d1.l }", D[0], D[2]); break;
    case 0x800B: snprintf(event, 256, "Ev$SetR+Ev$All { d0.l:%d d2.l:%d } => { d1.l }", D[0], D[2]); break;
    default: snprintf(event, 256, "Unknown event 0x%X", word);
    }
    return event;
}

inline std::string disassembleOS9Call(const uint16_t word, const uint32_t D[8], const uint32_t A[8], const std::function<const uint8_t*(const uint32_t)> get)
{
    char call[256] = {0};
    switch(word)
    {
    case 0x0000: snprintf(call, 256, "F$Link { (a0).s:\"%s\" d0.w:%hd } => { (a0).s a1 a2 d0.w d1.w }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""), D[0]); break;
    case 0x0001: snprintf(call, 256, "F$Load { (a0).s:\"%s\" d0.b:%hhd d1.l:%d } => { (a0).s a1 a2 d0.w d1.w }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""), D[0], D[1]); break;
    case 0x0002: snprintf(call, 256, "F$UnLink { a2:0x%X }", A[2]); break;
    case 0x0003: snprintf(call, 256, "F$Fork { (a0).s:\"%s\" a1:0x%X d0.w:%hd d1.l:%d d2.l:%d d3.w:%hd d4.w:%hd } => { (a0).s d0.w }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""), A[1], D[0], D[1], D[2], D[3], D[4]); break;
    case 0x0004: snprintf(call, 256, "F$Wait {} => { d0.w:%hd d1.w:%hd }", D[0], D[1]); break;
    case 0x0005: snprintf(call, 256, "F$Chain { (a0).s:\"%s\" a1:0x%X d0.w:%hd d1.l:%d d2.l:%d d3.w:%hd d4.w:%hd }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""), A[1], D[0], D[1], D[2], D[3], D[4]); break;
    case 0x0006: snprintf(call, 256, "F$Exit { d1.w:%hd }", D[1]); break;
    case 0x0007: snprintf(call, 256, "F$Mem { d0.l:%d } => { d0.l a1 }", D[0]); break;
    case 0x0008: snprintf(call, 256, "F$Send { d0.w:%hd d1.w:%hd }", D[0], D[1]); break;
    case 0x0009: snprintf(call, 256, "F$Icpt { a0:0x%X a6:0x%X }", A[0], A[6]); break;
    case 0x000A: snprintf(call, 256, "F$Sleep { d0.l:%d } => { d0.l }", D[0]); break;
    case 0x000B: snprintf(call, 256, "F$SSpd { d0.w:%hd }", D[0]); break;
    case 0x000C: snprintf(call, 256, "F$ID {} => { d0.w d1.l d2.w }"); break;
    case 0x000D: snprintf(call, 256, "F$SPrior { d0.w:%hd d1.w:%hd }", D[0], D[1]); break;
    case 0x000E: snprintf(call, 256, "F$STrap { a0:0x%X a1:0x%X }", A[0], A[1]); break;
    case 0x000F: snprintf(call, 256, "F$PErr { d0.w:%hd d1.w:%hd }", D[0], D[1]); break;
    case 0x0010: snprintf(call, 256, "F$PrsNam { (a0).s:\"%s\" } => { a0 d0.b d1.w a1 }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)"")); break;
    case 0x0011: snprintf(call, 256, "F$CmpNam { (a0).s:\"%s\" (a1).s:\"%s\" d1.w:%hd }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""), (get(A[1]) != nullptr ? get(A[1]) : (uint8_t*)""), D[1]); break;
    case 0x0012: snprintf(call, 256, "F$SchBit { d0.w:%hd d1.w:%hd a0:0x%X a1:0x%X } => { d0.w d1.w }", D[0], D[1], A[0], A[1]); break;
    case 0x0013: snprintf(call, 256, "F$AllBit { a0:0x%X d0.w:%hd d1.w:%hd }", A[0], D[0], D[1]); break;
    case 0x0014: snprintf(call, 256, "F$DelBit { a0:0x%X d0.w:%hd d1.w:%hd }", A[0], D[0], D[1]); break;
    case 0x0015: snprintf(call, 256, "F$Time { d0.w:%hd } => { d0.l d1.l d2.w d3.l }", D[0]); break;
    case 0x0016: snprintf(call, 256, "F$STime { d0.l:%d d1.l:%d }", D[0], D[1]); break;
    case 0x0017: snprintf(call, 256, "F$CRC { a0:0x%X d0.l:%d d1.l:%d } => { d1.l }", A[0], D[0], D[1]); break;
    case 0x0018: snprintf(call, 256, "F$GPrDsc { d0.w:%hd d1.w:%hd a0:0x%X }", D[0], D[1], A[0]); break;
    case 0x0019: snprintf(call, 256, "F$GBlkMp { d0.l:%d d1.l:%d a0:0x%X } => { d0.l d1.l d2.l d3.l a0 }", D[0], D[1], A[0]); break;
    case 0x001A: snprintf(call, 256, "F$GModDr { a0:0x%X d1.l:%d } => { d1.l }", A[0], D[1]); break;
    case 0x001B: snprintf(call, 256, "F$CpyMem { a0:0x%X a1:0x%X d0.w:%hd d1.l:%d }", A[0], A[1], D[0], D[1]); break;
    case 0x001C: snprintf(call, 256, "F$SUser { d1.l:%d }", D[1]); break;
    case 0x001D: snprintf(call, 256, "F$UnLoad { (a0).s:\"%s\" d0.w:%hd } => { (a0).s }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""), D[0]); break;
    case 0x001E: snprintf(call, 256, "F$RTE"); break;
    case 0x001F: snprintf(call, 256, "F$GPrDBT { d1.l:%d a0:0x%X } => { d1.l }", D[1], A[0]); break;
    case 0x0020: snprintf(call, 256, "F$Julian { d0.l:%d d1.l:%d } => { d0.l d1.l }", D[0], D[1]); break;
    case 0x0021: snprintf(call, 256, "F$TLink { (a0).s:\"%s\" d0.w:%hd } => { (a0).s d0.w d1.w a1 a2 }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""), D[0]); break;
    case 0x0022: snprintf(call, 256, "F$DFork { (a0).s:\"%s\" a1:0x%X a2:0x%X d0.w:%hd d1.l:%d d2.l:%d d3.w:%hd d4.w:%hd } => { (a0).s a2 d0.w }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""), A[1], A[2], D[0], D[1], D[2], D[3], D[4]); break;
    case 0x0023: snprintf(call, 256, "F$DExec { d0.w:%hd d1.l:%d d2.w:%hd a0:0x%X } => { d0.l d1.l d2.w d3.w d4.l d5.w }", D[0], D[1], D[2], A[0]); break;
    case 0x0024: snprintf(call, 256, "F$DExit { d0.w:%hd }", D[0]); break;
    case 0x0025: snprintf(call, 256, "F$DatMod { (a0).s:\"%s\" d0.l:%d d1.w:%hd d2.w:%hd d3.w:%hd d4.l:%d } => { (a0).s d0.w d1.w a1 a2 }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""), D[0], D[1], D[2], D[3], D[4]); break;
    case 0x0026: snprintf(call, 256, "F$SetCRC { a0:0x%X }", A[0]); break;
    case 0x0027: snprintf(call, 256, "F$SetSys { d0.w:%hd d1.l:%d d2.l:%d } => { d2.l }", D[0], D[1], D[2]); break;
    case 0x0028: snprintf(call, 256, "F$SRqMem { d0.l:%d } => { d0.l a2 }", D[0]); break;
    case 0x0029: snprintf(call, 256, "F$SRtMem { a2:0x%X d0.l:%d }", A[2], D[0]); break;
    case 0x002A: snprintf(call, 256, "F$IRQ { a0:0x%X a2:0x%X a3:0x%X d0.b:%hhd d1.b:%hhd }", A[0], A[2], A[3], D[0], D[1]); break;
    case 0x002B: snprintf(call, 256, "F$IOQu { d0.w:%hd }", D[0]); break;
    case 0x002C: snprintf(call, 256, "F$AProc { a0:0x%X }", A[0]); break;
    case 0x002D: snprintf(call, 256, "F$NProc"); break;
    case 0x002E: snprintf(call, 256, "F$VModul { a0:0x%X d0.l:%d d1.l:%d } => { a2 }", A[0], D[0], D[1]); break;
    case 0x002F: snprintf(call, 256, "F$FindPD { a0:0x%X d0.w:%hd } => { a1 }", A[0], D[0]); break;
    case 0x0030: snprintf(call, 256, "F$AllPD { a0:0x%X } => { a1 d0.w }", A[0]); break;
    case 0x0031: snprintf(call, 256, "F$RetPD { a0:0x%X d0.w:%hd }", A[0], D[0]); break;
    case 0x0032: snprintf(call, 256, "F$SSvc { a1:0x%X a3:0x%X }", A[1], A[3]); break;
    case 0x0033: snprintf(call, 256, "F$IODel { a0 }"); break;
    case 0x0037: snprintf(call, 256, "F$GProcP { d0.w } => { a1 }"); break;
    case 0x0038: snprintf(call, 256, "F$Move { a0:0x%X a2:0x%X d2.l:%d }", A[0], A[2], D[2]); break;
    case 0x0039: snprintf(call, 256, "F$AllRAM { xxx }"); break;
    case 0x003A: snprintf(call, 256, "F$Permit { d0.l d1.b a2 }"); break;
    case 0x003B: snprintf(call, 256, "F$Protect { d0.l d1.b a2 }"); break;
    case 0x003F: snprintf(call, 256, "F$AllTsk"); break;
    case 0x0040: snprintf(call, 256, "F$DelTsk"); break;
    case 0x004B: snprintf(call, 256, "F$AllPrc {} => { a2 }"); break;
    case 0x004C: snprintf(call, 256, "F$DelPrc { d0.w:%hd }", D[0]); break;
    case 0x004E: snprintf(call, 256, "F$FModul { xxx }"); break;
    case 0x0052: snprintf(call, 256, "F$SysDbg"); break;
    case 0x0053: snprintf(call, 256, "F$Event (%s) { d1.w:%hd }", disassembleOS9Event(D[1], D, A, get).c_str(), D[1]); break;
    case 0x0054: snprintf(call, 256, "F$Gregor { d0.l:%d d1.l:%d } => { d0.l d1.l }", D[0], D[1]); break;
    case 0x0055: snprintf(call, 256, "F$SysID { d0.l a0 a1 a2 a3 } => { d0.l d1.l d2.l d3.l d4.l d5.l d6.l d7.l (a0).s (a1).s (a2) (a3) }"); break;
    case 0x0056: snprintf(call, 256, "F$Alarm { d0.l:%d d1.w:%hd d2.l:%d d3.l:%d d4.l:%d a0:0x%X } => { d0.l }", D[0], D[1], D[2], D[3], D[4], A[0]); break;
    case 0x0057: snprintf(call, 256, "F$SigMask { d0.l:%d d1.l:%d }", D[0], D[1]); break;
    case 0x0058: snprintf(call, 256, "F$ChkMem { d0.l d1.b a2 }"); break;
    case 0x0059: snprintf(call, 256, "F$UAcct { d0.w:%hd a0:0x%X }", D[0], A[0]); break;
    case 0x005A: snprintf(call, 256, "F$CCtl { d0.l:%d }", D[0]); break;
    case 0x005B: snprintf(call, 256, "F$GSPUMp { d0.w d2.l a0 } => { d2.l (a0) }"); break;
    case 0x005C: snprintf(call, 256, "F$SRqCMem { d0.l:%d d1.l:%d } => { d0.l a2 }", D[0], D[1]); break;
    case 0x005D: snprintf(call, 256, "F$POSK { xxx }"); break;
    case 0x005E: snprintf(call, 256, "F$Panic { d0.l:%d }", D[0]); break;
    case 0x005F: snprintf(call, 256, "F$MBuf { xxx }"); break;
    case 0x0060: snprintf(call, 256, "F$Trans { d0.l:%d d1.l:%d a0:0x%X } => { d0.l a0 }", D[0], D[1], A[0]); break;
    case 0x0080: snprintf(call, 256, "I$Attach { (a0).s:\"%s\" d0.b:%hhd } => { a2 }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""), D[0]); break;
    case 0x0081: snprintf(call, 256, "I$Detach { a2:0x%X }", A[2]); break;
    case 0x0082: snprintf(call, 256, "I$Dup { d0.w:%hd } => { d0.w }", D[0]); break;
    case 0x0083: snprintf(call, 256, "I$Create { (a0).s:\"%s\" d0.b:%hhd d1.w:%hd d2.l:%d } => { (a0).s d0.w }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""), D[0], D[1], D[2]); break;
    case 0x0084: snprintf(call, 256, "I$Open { (a0).s:\"%s\" d0.b:%hhd } => { d0.w a0 }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""), D[0]); break;
    case 0x0085: snprintf(call, 256, "I$MakDir { (a0).s:\"%s\" d0.b:%hhd d1.w:%hd d2.l:%d } => { (a0).s }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""), D[0], D[1], D[2]); break;
    case 0x0086: snprintf(call, 256, "I$ChgDir { (a0).s:\"%s\" d0.b:%hhd } => { (a0).s }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""), D[0]); break;
    case 0x0087: snprintf(call, 256, "I$Delete { (a0).s:\"%s\" d0.b:%hhd } => { (a0).s }", (get(A[0]) != nullptr ? get(A[0]) : (uint8_t*)""), D[0]); break;
    case 0x0088: snprintf(call, 256, "I$Seek { d0.w:%hd d1.l:%d }", D[0], D[1]); break;
    case 0x0089: snprintf(call, 256, "I$Read { d0.w:%hd d1.l:%d a0:0x%X } => { d1.l }", D[0], D[1], A[0]); break;
    case 0x008A: snprintf(call, 256, "I$Write { d0.w:%hd d1.l:%d a0:0x%X } => { d1.l }", D[0], D[1], A[0]); break;
    case 0x008B: snprintf(call, 256, "I$ReadLn { d0.w:%hd d1.l:%d a0:0x%X } => { d1.l }", D[0], D[1], A[0]); break;
    case 0x008C: snprintf(call, 256, "I$WritLn { d0.w:%hd d1.l:%d a0:0x%X } => { d1.l }", D[0], D[1], A[0]); break;
    case 0x008D: snprintf(call, 256, "I$GetStt { d0.w:%hd d1.w:%hd }", D[0], D[1]); break;
    case 0x008E: snprintf(call, 256, "I$SetStt { d0.w:%hd d1.w:%hd }", D[0], D[1]); break;
    case 0x008F: snprintf(call, 256, "I$Close { d0.w:%hd }", D[0]); break;
    default: snprintf(call, 256, "Unknown call 0x%X", word);
    }
    return call;
}

} // namespace OS9

#endif // OS9_UTILS_HPP
