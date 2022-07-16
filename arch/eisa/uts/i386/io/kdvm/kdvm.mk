#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)eisa:uts/i386/io/kdvm/kdvm.mk	1.3"

INCRT = ../..
CFLAGS = -O -I$(INCRT) -D_KERNEL -DSYSV $(MORECPP)
FRC =
NET=
NET2=
NET3=
BUS=AT386

COMFILES=$(CONF)/pack.d/kdvm/Driver.o

all:	$(COMFILES)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(COMFILES) 

$(CONF)/pack.d/kdvm/Driver.o:	kdvm.o
	$(LD) -r -o $@ kdvm.o


#
# Header dependencies
#

kdvm.o:	kdvm.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/ws/chan.h \
	$(INCRT)/sys/vid.h \
	$(INCRT)/sys/vdc.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/ddi.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/xque.h \
	$(FRC)

