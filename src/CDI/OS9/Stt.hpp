#ifndef CDI_OS9_STT_HPP
#define CDI_OS9_STT_HPP

#include <string>

namespace OS9
{

enum class FileManagerType
{
    DT_SCF  = 0,  // sequential character file type
    DT_RBF  = 1,  // random block file type
    DT_PIPE = 2,  // pipe file type
    DT_SBF  = 3,  // sequential block file type
    DT_NFM  = 4,  // network file type
    DT_CDFM = 5,  // compact disc file type
    DT_UCM  = 6,  // user communication manager
    DT_SOCK = 7,  // socket communication manager
    DT_PTTY = 8,  // pseudo-keyboard manager
    DT_INET = 9,  // internet interface manager
    DT_NRF  = 10, // non-volatile ram file manager (CD-I variety)
    DT_GFM  = 11, // graphics file manager
};

enum class SttFunction
{
    SS_Opt = 0x00, // read/write PD options
    SS_Ready = 0x01, // check for device ready
    SS_Size = 0x02, // read/write file size
    SS_Reset = 0x03, // device restore
    SS_WTrk = 0x04, // device write track
    SS_Pos = 0x05, // get file current position
    SS_EOF = 0x06, // test for end of file
    SS_Link = 0x07, // link to status routines
    SS_ULink = 0x08, // unlink status routines
    SS_Feed = 0x09, // destructive forward skip (form feed)
    SS_Frz = 0x0a, // freeze DD_ information
    SS_SPT = 0x0b, // set DD_TKS to given value
    SS_SQD = 0x0c, // sequence down hard disk
    SS_DCmd = 0x0d, // send direct command to device
    SS_DevNm = 0x0e, // return device name
    SS_FD = 0x0f, // return file descriptor
    SS_Ticks = 0x10, // set lockout honor duration
    SS_Lock = 0x11, // lock/release record
    SS_DStat = 0x12, // return display status
    SS_Joy = 0x13, // return joystick value
    SS_BlkRd = 0x14, // block read
    SS_BlkWr = 0x15, // block write
    SS_Reten = 0x16, // retention cycle
    SS_WFM = 0x17, // write file mark
    SS_RFM = 0x18, // read past file mark
    SS_ELog = 0x19, // read error log
    SS_SSig = 0x1a, // send signal on data ready
    SS_Relea = 0x1b, // release device
    SS_Attr = 0x1c, // set file attributes
    SS_Break = 0x1d, // send break out serial device
    SS_RsBit = 0x1e, // reserve bitmap sector (for disk reorganization)
    SS_RMS = 0x1f, // get/set Motorola RMS status
    SS_FDInf = 0x20, // get FD info for specified FD sector
    SS_ACRTC = 0x21, // get/set Hitachi ACRTC status
    SS_IFC = 0x22, // serial input flow control
    SS_OFC = 0x23, // serial output flow control
    SS_EnRTS = 0x24, // enable RTS (modem control)
    SS_DsRTS = 0x25, // disable RTS (modem control)
    SS_DCOn = 0x26, // send signal DCD TRUE
    SS_DCOff = 0x27, // send signal DCD FALSE
    SS_Skip = 0x28, // skip block(s)
    SS_Mode = 0x29, // set RBF access mode
    SS_Open = 0x2a, // notification of new path opened
    SS_Close = 0x2b, // notification of path being closed

    SS_Path = 0x2c, // CDFM return pathlist for open path
    SS_Play = 0x2d, // CDFM play (CD-I) file
    SS_HEADER = 0x2e, // CDFM return header of last sector read
    SS_Raw = 0x2f, // CDFM read raw sectors
    SS_Seek = 0x30, // CDFM issue physical seek command
    SS_Abort = 0x31, // CDFM abort asynchronous operation in progress
    SS_CDDA = 0x32, // CDFM play CD digital audio
    SS_Pause = 0x33, // CDFM pause the disc driver
    SS_Eject = 0x34, // CDFM open the drive door
    SS_Mount = 0x35, // CDFM mount disc by disc number
    SS_Stop = 0x36, // CDFM stop the disc drive
    SS_Cont = 0x37, // CDFM start the disc after pause
    SS_Disable = 0x38, // CDFM disable hardware controls
    SS_Enable = 0x39, // CDFM enable hardware controls
    SS_ReadToc = 0x3a, // CDFM read TOC (on red discs)
    SS_SM = 0x3b, // CDFM's soundmap control status code
    SS_SD = 0x3c, // CDFM's sound data manipulation status code
    SS_SC = 0x3d, // CDFM's sound control status code

    SS_SEvent = 0x3E, // set event on data ready
    SS_Sound = 0x3F, // produce audible sound
    SS_DSize = 0x40, // get drive size (in sectors)
    SS_Net = 0x41, // NFM wild card getstat/setstat, with subcode
    SS_Rename = 0x42, // rename file
    SS_Free = 0x43, // return free statistics
    SS_VarSect = 0x44, // variable sector size supported query

    SS_UCM = 0x4C, // UCM reserved
    SS_DM = 0x51, // UCM's drawmap control status code
    SS_GC = 0x52, // UCM's graphics cursor status code
    SS_RG = 0x53, // UCM's region status code
    SS_DP = 0x54, // UCM's drawing parameters status code
    SS_DR = 0x55, // UCM's graphics drawing status code
    SS_DC = 0x56, // UCM's display control status code
    SS_CO = 0x57, // UCM's character output status code
    SS_VIQ = 0x58, // UCM's video inquiry status code
    SS_PT = 0x59, // UCM's pointer status code
    SS_SLink = 0x5A, // UCM link external subroutine module to UCM
    SS_KB = 0x5B, // keyboard status code

    // sockets
    SS_Bind = 0x6C, // bind a socket name
    SS_Listen = 0x6D, // listen for connections
    SS_Connect = 0x6E, // initiate a connection
    SS_Resv = 0x6F, // socket characteristics specification
    SS_Accept = 0x70, // accept socket connections
    SS_Recv = 0x71, // receive data
    SS_Send = 0x72, // send data
    SS_GNam = 0x73, // get socket name
    SS_SOpt = 0x74, // set socket option
    SS_GOpt = 0x75, // get socket option
    SS_Shut = 0x76, // shut down socket connection
    SS_SendTo = 0x77, // send to address
    SS_RecvFr = 0x78, // receive from address
    SS_Install = 0x79, // install upper level protocol (ULP)

    SS_PCmd = 0x7A, // protocol direct command

    SS_SN = 0x8C, // DSM's screen functions
    SS_AR = 0x8D, // DSM's action region functions
    SS_MS = 0x8E, // DSM's message functions
    SS_AC = 0x8F, // DSM's action cursor functions
    SS_CDFD = 0x90, // CDFM return file descriptor information
    SS_CCHAN = 0x91, // CDFM change channel request
};

std::string sttFunctionToString(const SttFunction stt);

} // namespace OS9

#endif // CDI_OS9_STT_HPP
