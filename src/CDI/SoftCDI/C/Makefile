#
# makefile for util utility
#
OBJ     = sysgo
MWOS    = -mw=C:\Users\User\MEGA\Dev\os9_68k_sdk_v12-master
# ROOT = C:\Users\User\MEGA\Dev\os9_68k_sdk_v12-master\OS9\68000
# RFILES = util.r util1.r util2.r util3.r util4.r util5.r util6.r
RDIR    = RELS
TP      = -tp=68070
# LIBPATH = -w=$(ROOT)\LIB
CFLAGS  = $(TP) -cw -s=4 -t=2 -b -h
# CFLAGS = $(TP) -v=$(ROOT)/DEFS -s=4 -t=2 -b
LFLAGS  = -olM=8k

$(OBJ): #$(RFILES)
    xcc $(MWOS) $(CFLAGS) $(LFLAGS)

# $(RFILES): utildefs.h
