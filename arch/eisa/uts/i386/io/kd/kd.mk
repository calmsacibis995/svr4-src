#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)eisa:uts/i386/io/kd/kd.mk	1.3"

INCRT = ../..
CFLAGS = -O -I$(INCRT) -D_KERNEL -DSYSV $(MORECPP)
FRC =
NET=
NET2=
NET3=
BUS=AT386

COMFILES=$(CONF)/pack.d/kd/Driver.o

all:	$(COMFILES)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(COMFILES) 

$(CONF)/pack.d/kd/Driver.o:	kdv.o kdkb.o evc.o vdc.o kdstr.o vtables.o kdvt.o
	$(LD) -r -o $@ kdstr.o kdkb.o kdv.o evc.o vdc.o vtables.o kdvt.o


#
# Header dependencies
#

kdkb.o:	kdkb.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/xque.h \
	$(INCRT)/sys/ws/8042.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/ddi.h \
	$(INCRT)/sys/kb.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/param.h \
	$(FRC)

kdstr.o:	kdstr.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/cram.h \
	$(INCRT)/sys/pic.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/v86.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/xdebug.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/ws/chan.h \
	$(INCRT)/sys/char.h \
	$(INCRT)/sys/ws/tcl.h \
	$(INCRT)/sys/ddi.h \
	$(INCRT)/sys/kb.h \
	$(FRC)

kdvt.o:	kdvt.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/cram.h \
	$(INCRT)/sys/pic.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/v86.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/xdebug.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/ws/chan.h \
	$(INCRT)/sys/char.h \
	$(INCRT)/sys/ws/tcl.h \
	$(INCRT)/sys/ddi.h \
	$(INCRT)/sys/kb.h \
	$(FRC)

kdv.o:	kdv.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/user.h \
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
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/v86.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/xdebug.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/vid.h \
	$(INCRT)/sys/vdc.h \
	$(FRC)

vtables.o:	vtables.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/vdc.h \
	$(FRC)

vdc.o:	vdc.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/cram.h \
	$(INCRT)/sys/pic.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/v86.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/xdebug.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/vid.h \
	$(INCRT)/sys/vdc.h \
	$(FRC)
evc.o:	evc.c \
 	$(INCRT)/sys/param.h \
 	$(INCRT)/sys/types.h \
 	$(INCRT)/sys/errno.h \
 	$(INCRT)/sys/kd.h \
 	$(INCRT)/sys/vid.h \
 	$(INCRT)/sys/evc.h

