#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-io:ws/ws.mk	1.3"

INCRT = ../..
CFLAGS = -O -I	$(INCRT) -D_KERNEL -DSYSV 	$(MORECPP)
FRC =
NET=
NET2=
NET3=
BUS=AT386
ARCH=AT386
WSDRIVER = $(CONF)/pack.d/ws/Driver.o

OBJS =	ws_cmap.o \
	ws_ansi.o \
	ws_subr.o \
	ws_tcl.o \
	ws_tables.o \
	ws_8042.o

all:	$(WSDRIVER)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f 	$(CONF)/pack.d/ws/Driver.o

FRC:


$(WSDRIVER):	$(OBJS)
	$(LD) -r -o 	$(WSDRIVER) $(OBJS)

ws_cmap.o:	ws_cmap.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/ascii.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/xque.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/ws/chan.h \
	$(FRC)

ws_ansi.o:	ws_ansi.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/ascii.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/xque.h \
	$(INCRT)/sys/ws/ws.h \
	$(FRC)

ws_subr.o:	ws_subr.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/pic.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/ascii.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/xque.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/ws/chan.h \
	$(INCRT)/sys/vid.h \
	$(INCRT)/sys/jioctl.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/session.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/ddi.h \
	$(FRC)

ws_tcl.o:	ws_tcl.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/ascii.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/xque.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/ws/chan.h \
	$(INCRT)/sys/ws/tcl.h \
	$(FRC)

ws_tables.o:	ws_tables.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/ascii.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/xque.h \
	$(INCRT)/sys/ws/ws.h \
	$(FRC)

ws_8042.o:	ws_8042.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/bootinfo.h \
	$(INCRT)/sys/kb.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/ws/8042.h \
	$(FRC)
