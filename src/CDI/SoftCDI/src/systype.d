
* Memory list definitions
* I assumed it was predefined by the compiler but looks like it's not
MemType macro
	dc.w \1,\2,\3,\4	type, priority, access, search block size
	dc.l \5,\6			low, high limits (where it appears on local address bus)
	dc.w \7,0			offset to description string (zero if none), reserved
	dc.l \8,0,0			address translation adjustment (for DMA, etc.), reserved
	ifne \#-8
		fail wrong number of arguments to MemType macro
	endc
	endm

CONFIG macro
MainFram dc.b "Philips CD-I",0
SysStart dc.b "launcher",0 name of initial module to execute
SysParam equ 0

SysDev equ 0 initial system disk pathlist
ConsolNm dc.b "/t2",0 console terminal pathlist
ClockNm dc.b "sgstom",0 clock module name
Extens dc.b "OS9P2 csdinit",0

	align
MemList MemType VIDEO1,128,B_USER,ProbeSize,PlaneABeg,PlaneAEnd,PlaneAVideo,0
		MemType VIDEO2,128,B_USER,ProbeSize,PlaneBBeg,PlaneBEnd,PlaneBVideo,0
* In case I support MPEG extension memory: put it here.
*	MemType SYSRAM,129,B_USER,ProbeSize,CPUBeg,BootMemEnd,OnBoard,CPUBeg+TRANS

	dc.l 0									terminate list

ProbeSize	equ $8000		Search every 1KB of memory
PlaneABeg	equ $500
PlaneAEnd	equ $80000
PlaneAVideo	dc.b "Plane A Video",0

PlaneBBeg	equ $200500
PlaneBEnd	equ $280000
PlaneBVideo	dc.b "Plane B Video",0
	endm
