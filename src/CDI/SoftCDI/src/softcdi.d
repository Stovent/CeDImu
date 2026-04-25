* SoftCDI system calls values (must mastch SoftCDIScheduler.hpp)
SoftCDI_Debug	equ	$100
CdDrive_Play	equ	$101
CdDrive_Stop	equ	$102
CdDrive_CopySector	equ	$103
CdDrive_GetSubheader	equ	$104
CdfmDeviceDriver_GetStat	equ	$105
CdfmDeviceDriver_SetStat	equ	$106

PointerDeviceDriver_GetPacket	equ $111
PointerDeviceDriver_GetStat		equ $112
PointerDeviceDriver_SetStat		equ $113

* CDFM device driver useful data
CdDrive_Port		equ	0
CdDrive_Vector		equ	204
CdDrive_Priority	equ	32

* MCD212 IRQ
MCD212_Port			equ	0
MCD212_Vector		equ	61	on-chip interrupt 5
MCD212_IRQLevel		equ	5
MCD212_Priority		equ	2

* Slave IRQ
Slave_Port			equ	$310000
Slave_Vector		equ 26
Slave_IRQLevel		equ	2
Slave_Priority		equ	1

* SoftCDI Pointer device
Pointer_Port		equ	0
Pointer_Vector		equ	201
Pointer_Priority	equ	32
