* Macro to invoke the SoftCDI process
softcdi	macro
		trap #0
		dc.w \1
		endm
