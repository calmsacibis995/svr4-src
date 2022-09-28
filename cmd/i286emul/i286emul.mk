#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)i286emu:i286emul.mk	1.1"

ROOT =
INCRT = $(ROOT)/usr/include
CFLAGS = -O -I$(INCRT) -D_STYPES
LDFLAGS = -s
CC = cc
# Warning, the kernel expects i286emul to be in /bin!
INSDIR = $(ROOT)/usr/bin
INS = install

OFILES1 = main.o utils.o run286.o setdscr.o
OFILES2 = exec.o gethead.o text.o
OFILES4 = sendsig.o Signal.o
OFILES3 = Sbreak.o ipc.o miscsys.o Sioctl.o
OFILES5 = sysent.o syscall.o syscalla.o

OFILES = $(OFILES1) $(OFILES2) $(OFILES3) $(OFILES4) $(OFILES5)

i286emul: $(OFILES)
	$(CC) $(CFLAGS) $(OFILES) -o i286emul $(LDFLAGS) $(SHLIBS)

$(OFILES):      vars.h

install:        i286emul
	$(INS) -f $(INSDIR) -m 755 -u bin -g bin i286emul

clobber:        clean
	rm -f i286emul

clean:
	rm -f $(OFILES)
