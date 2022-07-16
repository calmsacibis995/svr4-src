#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)at:uts/i386/io/kd/kd.mk	1.3.1.1"

INCRT = ../..
CFLAGS = -O -I$(INCRT) -D_KERNEL -DSYSV $(MORECPP)

KDFLAGS=$(CFLAGS)

FRC =
NET=
NET2=
NET3=
BUS=AT386
ARCH=AT386

COMFILES=$(CONF)/pack.d/kd/Driver.o

BLTOBJS=btc.o
EVCOBJS=evc.o
REQOBJS=kdv.o kdkb.o vdc.o kdstr.o vtables.o kdvt.o
EVGAOBJS=evga.o

OBJS=$(REQOBJS) $(BLTOBJS) $(EVCOBJS) $(EVGAOBJS)


all:	$(COMFILES)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(COMFILES) 

$(CONF)/pack.d/kd/Driver.o:	$(OBJS)
	$(LD) -r -o $@ $(OBJS)


#
# Header dependencies
#

kdkb.o:	kdkb.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/xque.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/ws/8042.h \
	$(INCRT)/sys/kb.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/ddi.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)
	$(CC) -c $(KDFLAGS) kdkb.c

kdstr.o:	kdstr.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/cram.h \
	$(INCRT)/sys/pic.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/xque.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/ws/chan.h \
	$(INCRT)/sys/ws/tcl.h \
	$(INCRT)/sys/jioctl.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/kb.h \
	$(INCRT)/sys/vid.h \
	$(INCRT)/sys/xdebug.h \
	$(INCRT)/sys/ddi.h \
	kd_btc.h \
	$(FRC)
	$(CC) -c $(KDFLAGS) kdstr.c

kdvt.o:	kdvt.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/xque.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/ws/chan.h \
	$(INCRT)/sys/vid.h \
	$(INCRT)/sys/ws/tcl.h \
	$(INCRT)/sys/kb.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/vdc.h \
	$(INCRT)/sys/ddi.h \
	$(FRC)
	$(CC) -c $(KDFLAGS) kdvt.c

kdv.o:	kdv.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/cram.h \
	$(INCRT)/sys/pic.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/xque.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/vid.h \
	$(INCRT)/sys/vdc.h \
	kd_btc.h \
	$(FRC)
	$(CC) -c $(KDFLAGS) kdv.c

vtables.o:	vtables.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/vid.h \
	$(INCRT)/sys/vdc.h \
	$(FRC)
	$(CC) -c $(KDFLAGS) vtables.c

vdc.o:	vdc.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/xque.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/vid.h \
	$(INCRT)/sys/vdc.h \
	$(FRC)
	$(CC) -c $(KDFLAGS) vdc.c

evc.o:	evc.c \
 	$(INCRT)/sys/param.h \
 	$(INCRT)/sys/types.h \
 	$(INCRT)/sys/errno.h \
 	$(INCRT)/sys/kd.h \
 	$(INCRT)/sys/vid.h \
 	$(INCRT)/sys/evc.h \
	$(FRC)
	$(CC) -c $(KDFLAGS) evc.c

btc.o:	btc.c \
	btc.h \
	$(FRC)
	$(CC) -c $(KDFLAGS) btc.c

evga.o:	evga.c \
 	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/xque.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/vid.h \
	$(INCRT)/sys/vdc.h \
	$(INCRT)/sys/inline.h \
	$(FRC)
	$(CC) -c $(KDFLAGS) evga.c
