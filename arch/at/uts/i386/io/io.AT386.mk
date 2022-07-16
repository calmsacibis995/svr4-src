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

#ident	"@(#)at:uts/i386/io/io.AT386.mk	1.3"

INCRT = ..
CFLAGS = -O -I$(INCRT) -D_KERNEL -DSYSV -DSVR40 $(MORECPP)
FRC =
NET=
NET2=
NET3=
BUS=AT386
ARCH=AT386

AT386FILES =\
	$(CONF)/pack.d/rtc/Driver.o	\
	$(CONF)/pack.d/hd/Driver.o	\
	$(CONF)/pack.d/cram/Driver.o	\
	$(CONF)/pack.d/fd/Driver.o	\
	$(CONF)/pack.d/kd/Driver.o	\
	$(CONF)/pack.d/kdvm/Driver.o	\
	$(CONF)/pack.d/gvid/Driver.o	\
	$(CONF)/pack.d/cmux/Driver.o	\
	$(CONF)/pack.d/asy/Driver.o	\
	$(CONF)/pack.d/dma/Driver.o	\
	$(CONF)/pack.d/lp/Driver.o

all: $(AT386FILES)

clean:
	cd kd; $(MAKE) -f kd.mk "BUS=$(BUS)" "ARCH=$(ARCH)" "ROOT=$(ROOT)" clean
	cd kdvm; $(MAKE) -f kdvm.mk "BUS=$(BUS)" "ARCH=$(ARCH)" "ROOT=$(ROOT)" clean

clobber:	clean
	cd kd; $(MAKE) -f kd.mk "BUS=$(BUS)" "ARCH=$(ARCH)" "ROOT=$(ROOT)" "CONF=$(CONF)" clobber
	cd kdvm; $(MAKE) -f kdvm.mk "BUS=$(BUS)" "ARCH=$(ARCH)" "ROOT=$(ROOT)" "CONF=$(CONF)" clobber
	-rm -f $(AT386FILES)

FRC:

$(CONF)/pack.d/cmux/Driver.o:	chanmux.o
	$(LD) -r -o $@ chanmux.o

$(CONF)/pack.d/gvid/Driver.o:	genvid.o
	$(LD) -r -o $@ genvid.o

$(CONF)/pack.d/kd/Driver.o:	KD

KD:
	cd kd; $(MAKE) -f kd.mk "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "BUS=$(BUS)" "ARCH=$(ARCH)" "CONF=$(CONF)" "NET=$(NET)" "NET2=$(NET2)" "NET3=$(NET3)" "CFLAGS=$(CFLAGS) -I../.."

$(CONF)/pack.d/kdvm/Driver.o:	KDVM

KDVM:
	cd kdvm; $(MAKE) -f kdvm.mk "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "BUS=$(BUS)" "ARCH=$(ARCH)" "CONF=$(CONF)" "NET=$(NET)" "NET2=$(NET2)" "NET3=$(NET3)" "CFLAGS=$(CFLAGS) -I../.."

$(CONF)/pack.d/dma/Driver.o:	dma.o i8237A.o
	$(LD) -r -o $@ dma.o i8237A.o

$(CONF)/pack.d/asy/Driver.o:	asy.o
	$(LD) -r -o $@ asy.o

$(CONF)/pack.d/cram/Driver.o:	cram.o
	$(LD) -r -o $@ cram.o

$(CONF)/pack.d/fd/Driver.o:	fd.o
	$(LD) -r -o $@ fd.o

$(CONF)/pack.d/hd/Driver.o:	hd.o
	$(LD) -r -o $@ hd.o

$(CONF)/pack.d/lp/Driver.o:	lp.o
	$(LD) -r -o $@ lp.o

$(CONF)/pack.d/rtc/Driver.o:	rtc.o
	$(LD) -r -o $@ rtc.o

#
# Header dependencies
#

dma.o: dma.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/dma.h \
	$(INCRT)/sys/i8237A.h \
	$(FRC)

i8237A.o: i8237A.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/dma.h \
	$(INCRT)/sys/i8237A.h \
	$(FRC)

asy.o:	asy.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/asy.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/eucioctl.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/ddi.h \
	$(INCRT)/sys/v86.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/tty.h \
	$(FRC)

cram.o:	cram.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/cram.h \
	$(FRC)

hd.o:	hd.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/hd.h \
	$(INCRT)/sys/alttbl.h \
	$(INCRT)/sys/vtoc.h \
	$(INCRT)/sys/fdisk.h \
	$(INCRT)/sys/bootinfo.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

kd.o: kd.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cram.h \
	$(INCRT)/sys/pic.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/v86.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vnode.h \
	$(FRC)

kdkbtables.o: kdkbtables.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/ascii.h \
	$(FRC)

vtgen.o: vtgen.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vt.h \
	$(FRC)

lp.o:	lp.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/lp.h \
	$(FRC)

genvid.o: genvid.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/genvid.h \
	$(FRC)

chanmux.o:	chanmux.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/ascii.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/ws/chan.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/char.h \
	$(INCRT)/sys/ddi.h \
	$(FRC)
