#ifndef CDI_OS9_SYSTEMCALLS_HPP
#define CDI_OS9_SYSTEMCALLS_HPP

#include "../cores/SCC68070/SCC68070.hpp"

namespace OS9
{

enum class SystemCallType
{
    F$Link = 0x0000,
    F$Load = 0x0001,
    F$UnLink = 0x0002,
    F$Fork = 0x0003,
    F$Wait = 0x0004,
    F$Chain = 0x0005,
    F$Exit = 0x0006,
    F$Mem = 0x0007,
    F$Send = 0x0008,
    F$Icpt = 0x0009,
    F$Sleep = 0x000A,
    F$SSpd = 0x000B,
    F$ID = 0x000C,
    F$SPrior = 0x000D,
    F$STrap = 0x000E,
    F$PErr = 0x000F,
    F$PrsNam = 0x0010,
    F$CmpNam = 0x0011,
    F$SchBit = 0x0012,
    F$AllBit = 0x0013,
    F$DelBit = 0x0014,
    F$Time = 0x0015,
    F$STime = 0x0016,
    F$CRC = 0x0017,
    F$GPrDsc = 0x0018,
    F$GBlkMp = 0x0019,
    F$GModDr = 0x001A,
    F$CpyMem = 0x001B,
    F$SUser = 0x001C,
    F$UnLoad = 0x001D,
    F$RTE = 0x001E,
    F$GPrDBT = 0x001F,
    F$Julian = 0x0020,
    F$TLink = 0x0021,
    F$DFork = 0x0022,
    F$DExec = 0x0023,
    F$DExit = 0x0024,
    F$DatMod = 0x0025,
    F$SetCRC = 0x0026,
    F$SetSys = 0x0027,
    F$SRqMem = 0x0028,
    F$SRtMem = 0x0029,
    F$IRQ = 0x002A,
    F$IOQu = 0x002B,
    F$AProc = 0x002C,
    F$NProc = 0x002D,
    F$VModul = 0x002E,
    F$FindPD = 0x002F,
    F$AllPD = 0x0030,
    F$RetPD = 0x0031,
    F$SSvc = 0x0032,
    F$IODel = 0x0033,
    F$GProcP = 0x0037,
    F$Move = 0x0038,
    F$AllRAM = 0x0039,
    F$Permit = 0x003A,
    F$Protect = 0x003B,
    F$AllTsk = 0x003F,
    F$DelTsk = 0x0040,
    F$AllPrc = 0x004B,
    F$DelPrc = 0x004C,
    F$FModul = 0x004E,
    F$SysDbg = 0x0052,
    F$Event = 0x0053,
    F$Gregor = 0x0054,
    F$SysID = 0x0055,
    F$Alarm = 0x0056,
    F$SigMask = 0x0057,
    F$ChkMem = 0x0058,
    F$UAcct = 0x0059,
    F$CCtl = 0x005A,
    F$GSPUMp = 0x005B,
    F$SRqCMem = 0x005C,
    F$POSK = 0x005D,
    F$Panic = 0x005E,
    F$MBuf = 0x005F,
    F$Trans = 0x0060,
    I$Attach = 0x0080,
    I$Detach = 0x0081,
    I$Dup = 0x0082,
    I$Create = 0x0083,
    I$Open = 0x0084,
    I$MakDir = 0x0085,
    I$ChgDir = 0x0086,
    I$Delete = 0x0087,
    I$Seek = 0x0088,
    I$Read = 0x0089,
    I$Write = 0x008A,
    I$ReadLn = 0x008B,
    I$WritLn = 0x008C,
    I$GetStt = 0x008D,
    I$SetStt = 0x008E,
    I$Close = 0x008F,
};

enum class Event
{
    Ev$Link = 0x0000,
    Ev$UnLnk = 0x0001,
    Ev$Creat = 0x0002,
    Ev$Delet = 0x0003,
    Ev$Wait = 0x0004,
    Ev$WaitR = 0x0005,
    Ev$Read = 0x0006,
    Ev$Info = 0x0007,
    Ev$Signl = 0x0008,
    Ev$Signl_Ev$All = 0x8008,
    Ev$Pulse = 0x0009,
    Ev$Pulse_Ev$All = 0x8009,
    Ev$Set = 0x000A,
    Ev$Set_Ev$All = 0x800A,
    Ev$SetR = 0x000B,
    Ev$SetR_Ev$All = 0x800B,
};

struct SystemCall
{
    SystemCallType m_type; /**< The type of the system call. */
    std::string inputs; /**< The input parameters. */
    std::string outputs; /**< The output parameters. it is at the beginning of the call and has to be filled by the application when RTE is executed. */
};

std::string eventNameToString(const Event evt);
std::string eventInputsToString(const Event evt, const std::map<CPURegister, uint32_t>& regs, const std::function<const uint8_t*(const uint32_t)>& get);
std::string eventOutputsToString(const Event evt, const std::map<CPURegister, uint32_t>& regs, const std::function<const uint8_t*(const uint32_t)>& get);

std::string systemCallNameToString(const SystemCallType call);
std::string systemCallInputsToString(const SystemCallType call, const std::map<CPURegister, uint32_t>& regs, const std::function<const uint8_t*(const uint32_t)>& get);
std::string systemCallOutputsToString(const SystemCallType call, const std::map<CPURegister, uint32_t>& regs, const std::function<const uint8_t*(const uint32_t)>& get);

} // namespace OS9

#endif // CDI_OS9_SYSTEMCALLS_HPP
