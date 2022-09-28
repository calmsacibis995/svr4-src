#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft 
#	Corporation and should be treated as Confidential.

#ident	"@(#)x286emul:x286emul.mk	1.1"

#
#	x286emul:  Xenix 286 API environment emulator
#

# Setting DFLAG to -DDEBUG turns on a lot of debugging output.
# Setting -DTRACE turns on system call tracing when the environment
#	variable SYSTRACE is set to Y.
#
# DFLAG = -DDEBUG -DTRACE

ROOT =
INCRT = $(ROOT)/usr/include
# CFLAGS = -O -I. -I$(INCRT) -DAT386 -D_STYPES $(DFLAG)
CFLAGS = -I. -I$(INCRT) -DAT386 -D_STYPES $(DFLAG)
LDFLAGS = -s -lx
CC = cc
# Warning, the kernel expects x286emul to be in /usr/bin!
INSDIR = $(ROOT)/usr/bin
INS = install

TARGET = x286emul

SFILES = sendsig.s \
syscalla.s \
run286.s \
float.s \
main.c \
utils.c \
sysent.c \
setdscr.c \
syscall.c \
miscsys.c \
exec.c \
debug.c \
ioctl.c \
cxenix.c \
signal.c \
moresys.c \
msgsys.c \
shm.c

HFILES = vars.h \
sysent.h \
h/syscall.h

OFILES = sendsig.o \
syscalla.o \
run286.o \
float.o \
main.o \
utils.o \
sysent.o \
setdscr.o \
syscall.o \
miscsys.o \
exec.o \
debug.o \
ioctl.o \
cxenix.o \
signal.o \
moresys.o \
msgsys.o \
shm.o

all:	x286emul

x286emul:	$(OFILES)
	$(CC) $(OFILES) -o $(TARGET) $(LDFLAGS) $(SHLIBS)

main.o:	vars.h main.c

install:        x286emul
	$(INS) -f $(INSDIR) -m 755 -u bin -g bin x286emul
	-$(SYMLINK) $(INSDIR)/x286emul $(ROOT)/bin/x286emul

clobber:	clean
	rm -f $(TARGET)

clean:
	rm -f $(OFILES)
