#ifndef OS9_HPP
#define OS9_HPP

#include "utils.hpp"

inline std::string disassembleOS9Call(const uint16_t word)
{
    switch(word)
    {
    case 0x0000: return "F$Link : (a0).s d0.w : (a0).s a1 a2 d0.w d1.w";
    case 0x0001: return "F$Load : (a0).s d0.b d1.l : (a0).s a1 a2 d0.w d1.w";
    case 0x0002: return "F$UnLink : a2 : ";
    case 0x0003: return "F$Fork : (a0).s a1 d0.w d1.l d2.l d3.w d4.w : (a0).s d0.w";
    case 0x0004: return "F$Wait :  : d0.w d1.w";
    case 0x0005: return "F$Chain : (a0).s a1 d0.w d1.l d2.l d3.w d4.w : ";
    case 0x0006: return "F$Exit : d1.w : ";
    case 0x0007: return "F$Mem : d0.l : d0.l a1";
    case 0x0008: return "F$Send : d0.w d1.i : ";
    case 0x0009: return "F$Icpt : a0 a6 : ";
    case 0x000A: return "F$Sleep : d0.l : d0.l";
    case 0x000B: return "F$SSpd : d0.w : ";
    case 0x000C: return "F$ID :  : d0.w d1.l d2.w";
    case 0x000D: return "F$SPrior : d0.w d1.w : ";
    case 0x000E: return "F$STrap : a0 a1 : ";
    case 0x000F: return "F$PErr : d0.w d1.w : ";
    case 0x0010: return "F$PrsNam : (a0).s : a0 d0.b d1.w a1";
    case 0x0011: return "F$CmpNam : (a0).s (a1).s d1.w : ";
    case 0x0012: return "F$SchBit : d0.w d1.w a0 a1 : d0.w d1.w";
    case 0x0013: return "F$AllBit : a0 d0.w d1.w : ";
    case 0x0014: return "F$DelBit : a0 d0.w d1.w : ";
    case 0x0015: return "F$Time : d0.w : d0.l d1.l d2.w d3.l";
    case 0x0016: return "F$STime : d0.l d1.l : ";
    case 0x0017: return "F$CRC : a0 d0.l d1.l : d1.l";
    case 0x0018: return "F$GPrDsc : d0.w d1.w a0 : ";
    case 0x0019: return "F$GBlkMp : d0.l d1.l a0 : d0.l d1.l d2.l d3.l a0";
    case 0x001A: return "F$GModDr : a0 d1.l : d1.l";
    case 0x001B: return "F$CpyMem : a0 a1 d0.w d1.l : ";
    case 0x001C: return "F$SUser : d1.l : ";
    case 0x001D: return "F$UnLoad : (a0).s d0.w : (a0).s";
    case 0x001E: return "F$RTE :  : ";
    case 0x001F: return "F$GPrDBT : d1.l a0 : d1.l";
    case 0x0020: return "F$Julian : d0.l d1.l : d0.l d1.l";
    case 0x0021: return "F$TLink : (a0).s d0.w : (a0).s d0.w d1.w a1 a2";
    case 0x0022: return "F$DFork : (a0).s a1 a2 d0.w d1.l d2.l d3.w d4.w : (a0).s a2 d0.w";
    case 0x0023: return "F$DExec : d0.w d1.l d2.w a0 : d0.l d1.l d2.w d3.w d4.l d5.w";
    case 0x0024: return "F$DExit : d0.w : ";
    case 0x0025: return "F$DatMod : (a0).s d0.l d1.w d2.w d3.w d4.l : (a0).s d0.w d1.w a1 a2";
    case 0x0026: return "F$SetCRC : a0 : ";
    case 0x0027: return "F$SetSys : d0.w d1.l d2.l : d2.l";
    case 0x0028: return "F$SRqMem : d0.l : d0.l a2";
    case 0x0029: return "F$SRtMem : a2 d0.l : ";
    case 0x002A: return "F$IRQ : a0 a2 a3 d0.b d1.b : ";
    case 0x002B: return "F$IOQu : d0.w : ";
    case 0x002C: return "F$AProc : a0 : ";
    case 0x002D: return "F$NProc";
    case 0x002E: return "F$VModul : a0 d0.l d1.l : a2";
    case 0x002F: return "F$FindPD : a0 d0.w : a1";
    case 0x0030: return "F$AllPD : a0 : a1 d0.w";
    case 0x0031: return "F$RetPD : a0 d0.w : ";
    case 0x0032: return "F$SSvc : a1 a3 : ";
    case 0x0033: return "F$IODel : a0 : ";
    case 0x0037: return "F$GProcP : d0.w : a1";
    case 0x0038: return "F$Move : a0 a2 d2.l : ";
    case 0x0039: return "F$AllRAM : xxx : ";
    case 0x003A: return "F$Permit : d0.l d1.b a2 : ";
    case 0x003B: return "F$Protect : d0.l d1.b a2 : ";
    case 0x003F: return "F$AllTsk";
    case 0x0040: return "F$DelTsk";
    case 0x004B: return "F$AllPrc :  : a2";
    case 0x004C: return "F$DelPrc : d0.w : ";
    case 0x004E: return "F$FModul : xxx : ";
    case 0x0052: return "F$SysDbg";
    case 0x0053: return "F$Event : d1.w d1.w (a0).s : ";
    case 0x0054: return "F$Gregor : d0.l d1.l : d0.l d1.l";
    case 0x0055: return "F$SysID : d0.l a0 a1 a2 a3 : d0.l d1.l d2.l d3.l d4.l d5.l d6.l d7.l (a0).s (a1).s (a2) (a3)";
    case 0x0056: return "F$Alarm : d0.l d1.w d2.l d3.l d4.l sa0 : d0.l";
    case 0x0057: return "F$SigMask : d0.l d1.l : ";
    case 0x0058: return "F$ChkMem : d0.l d1.b a2 : ";
    case 0x0059: return "F$UAcct : d0.w a0 : ";
    case 0x005A: return "F$CCtl : d0.l : ";
    case 0x005B: return "F$GSPUMp : d0.w d2.l a0 : d2.l (a0)";
    case 0x005C: return "F$SRqCMem : d0.l d1.l : d0.l a2";
    case 0x005D: return "F$POSK : xxx : ";
    case 0x005E: return "F$Panic : d0.l : ";
    case 0x005F: return "F$MBuf : xxx : ";
    case 0x0060: return "F$Trans : d0.l d1.l a0 : d0.l a0";
    case 0x0080: return "I$Attach : (a0).s d0.b : a2";
    case 0x0081: return "I$Detach : a2 : ";
    case 0x0082: return "I$Dup : d0.w : d0.w";
    case 0x0083: return "I$Create : (a0).s d0.b d1.w d2.l : (a0).s d0.w";
    case 0x0084: return "I$Open : (a0).s d0.b : d0.w a0";
    case 0x0085: return "I$MakDir : (a0).s d0.b d1.w d2.l : (a0).s";
    case 0x0086: return "I$ChgDir : (a0).s d0.b : (a0).s";
    case 0x0087: return "I$Delete : (a0).s d0.b : (a0).s";
    case 0x0088: return "I$Seek : d0.w d1.h : ";
    case 0x0089: return "I$Read : d0.w d1.l a0 : d1.l";
    case 0x008A: return "I$Write : d0.w d1.l (a0).s : d1.l";
    case 0x008B: return "I$ReadLn : d0.w d1.l a0 : d1.l";
    case 0x008C: return "I$WritLn : d0.w d1.l (a0).s : d1.l";
    case 0x008D: return "I$GetStt : d0.w d1.w : ";
    case 0x008E: return "I$SetStt : d0.w d1.w : ";
    case 0x008F: return "I$Close : d0.w : ";
    default: return "Unknown call 0x" + toHex(word);
    }
}

#endif // OS9_HPP
