#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1988  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:uts/i386/io/io.MBUS.mk	1.3.2.1"

INCRT = ..
CFLAGS = -O -I$(INCRT) -D_KERNEL -DSYSV $(MORECPP)
FRC =
BUS	=	MB2
ARCH =	MBUS

FILES =\
	$(CONF)/pack.d/ics/Driver.o	\
	$(CONF)/pack.d/dma/Driver.o	\
	$(CONF)/pack.d/d258/Driver.o	\
	$(CONF)/pack.d/mpc/Driver.o	\
	$(CONF)/pack.d/mps/Driver.o	\
	$(CONF)/pack.d/mb2stai/Driver.o	\
	$(CONF)/pack.d/mb2tsm/Driver.o	\
	$(CONF)/pack.d/i258/Driver.o	\
	$(CONF)/pack.d/i258tp/Driver.o	\
	$(CONF)/pack.d/i410/Driver.o	\
	$(CONF)/pack.d/rci/Driver.o	\
	$(CONF)/pack.d/cci/Driver.o	\
	$(CONF)/pack.d/atcs/Driver.o	\
	$(CONF)/pack.d/iasy/Driver.o	\
	$(CONF)/pack.d/console/Driver.o \
	$(CONF)/pack.d/ramd/Driver.o \
	$(CONF)/pack.d/sysmsg/Driver.o   \
	$(CONF)/pack.d/clock/Driver.o \
	$(CONF)/pack.d/csmclk/Driver.o \
	$(CONF)/pack.d/bps/Driver.o   \
	$(CONF)/pack.d/i354/Driver.o \
	$(CONF)/pack.d/i350/Driver.o

all:	
	 $(MAKE) -f io.$(ARCH).mk $(FILES)
	 cd enetdrv; $(MAKE) -f enetdrv.mk "CFLAGS=$(CFLAGS)" "ROOT=$(ROOT)" "BUS=$(BUS)" "ARCH=$(ARCH)" 
	 cd ots;  $(MAKE) -f ots.mk  "CFLAGS=$(CFLAGS)" "ROOT=$(ROOT)" "BUS=$(BUS)" "ARCH=$(ARCH)" 

clean:

clobber:	clean
	-rm -f  $(FILES) 
	 cd enetdrv; $(MAKE) -f enetdrv.mk clobber
	 cd ots;  $(MAKE) -f ots.mk  clobber

FRC:

#
# Drivers
#

$(CONF)/pack.d/ics/Driver.o:	ics.o
	$(LD) -r -o $@ ics.o

$(CONF)/pack.d/dma/Driver.o:	dma.o
	$(LD) -r -o $@ dma.o

$(CONF)/pack.d/d258/Driver.o:	d258.o 
	$(LD) -r -o $@ d258.o 

$(CONF)/pack.d/mpc/Driver.o:	mpc.o
	$(LD) -r -o $@ mpc.o

$(CONF)/pack.d/mps/Driver.o:	mps.o
	$(LD) -r -o $@ mps.o

$(CONF)/pack.d/mb2tsm/Driver.o:	mb2tsm.o
	$(LD) -r -o $@ mb2tsm.o

$(CONF)/pack.d/mb2stai/Driver.o: mb2stai.o
	$(LD) -r -o $@ mb2stai.o

$(CONF)/pack.d/i258/Driver.o:	i258.o
	$(LD) -r -o $@ i258.o

$(CONF)/pack.d/i258tp/Driver.o:	i258tp.o
	$(LD) -r -o $@ i258tp.o

$(CONF)/pack.d/i410/Driver.o:	i410.o
	$(LD) -r -o $@ i410.o

$(CONF)/pack.d/rci/Driver.o:	rci.o
	$(LD) -r -o $@ rci.o

$(CONF)/pack.d/cci/Driver.o:	cci.o
	$(LD) -r -o $@ cci.o

$(CONF)/pack.d/atcs/Driver.o:	atcs.o
	$(LD) -r -o $@ atcs.o

$(CONF)/pack.d/iasy/Driver.o:	iasy.o
	$(LD) -r -o $@ iasy.o

$(CONF)/pack.d/console/Driver.o:	console.o
	$(LD) -r -o $@ console.o

$(CONF)/pack.d/clock/Driver.o:	clock.o
	$(LD) -r -o $@ clock.o

$(CONF)/pack.d/csmclk/Driver.o:	csmclk.o
	$(LD) -r -o $@ csmclk.o

$(CONF)/pack.d/i354/Driver.o:	i354.o
	$(LD) -r -o $@ i354.o

$(CONF)/pack.d/i350/Driver.o:	i350.o
	$(LD) -r -o $@ i350.o

$(CONF)/pack.d/ramd/Driver.o:	ramd.o
	$(LD) -r -o $@ ramd.o

$(CONF)/pack.d/sysmsg/Driver.o:	sysmsg.o
	$(LD) -r -o $@ sysmsg.o

$(CONF)/pack.d/bps/Driver.o:	bps.o
	[ -d $(CONF)/pack.d/bps ] || mkdir $(CONF)/pack.d/bps; \
	$(LD) -r -o $@ bps.o

#
# Header dependencies
#
ramd.o: ramd.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/ramd.h \
	$(FRC)

#
# MB2 Header dependencies
#

ics.o:	ics.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/ics.h \
	$(FRC)
 
mpc.o:	mpc.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/mpc.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/dma.h \
	$(INCRT)/sys/i82258.h \
	$(INCRT)/sys/ics.h \
	$(INCRT)/sys/mps.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/immu.h \
	$(FRC)
 
mps.o: mps.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/mpc.h \
	$(INCRT)/sys/dma.h \
	$(INCRT)/sys/mps.h \
	$(INCRT)/sys/ics.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/ddi.h \
	$(FRC)
 
dma.o: 	dma.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/dma.h \
	$(INCRT)/sys/i82258.h \
	$(FRC)
 
d258.o: d258.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/dma.h \
	$(INCRT)/sys/i82258.h \
	$(INCRT)/sys/mps.h \
	$(FRC)
 
mb2stai.o:	mb2stai.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strstat.h \
	$(INCRT)/sys/mps.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/mb2stai.h \
	$(INCRT)/sys/mb2taiusr.h \
	$(FRC)
 
mb2tsm.o:	mb2tsm.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strstat.h \
	$(INCRT)/sys/mps.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/mb2stai.h \
	$(INCRT)/sys/mb2taiusr.h \
	$(FRC)
 

iasy.o: iasy.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/mps.h \
	$(INCRT)/sys/ccimp.h \
	$(INCRT)/sys/iasy.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/ddi.h \
	$(FRC)
 
atcs.o: atcs.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/mps.h \
	$(INCRT)/sys/ccimp.h \
	$(INCRT)/sys/atcs.h \
	$(INCRT)/sys/atcsmp.h \
	$(INCRT)/sys/immu.h \
	$(FRC)
 
catcs.o: catcs.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/mps.h \
	$(INCRT)/sys/atcs.h \
	$(INCRT)/sys/atcsmp.h \
	$(FRC)
 
ccci.o: ccci.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/mps.h \
	$(INCRT)/sys/cci.h \
	$(FRC)
 
rci.o: rci.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/mps.h \
	$(INCRT)/sys/rcimp.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)
 
cci.o: cci.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/mps.h \
	$(INCRT)/sys/cci.h \
	$(INCRT)/sys/ccimp.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)
 
i410.o: i410.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/ics.h \
	$(INCRT)/sys/mps.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)
 
i258.o: i258.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/pit.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/ics.h \
	$(INCRT)/sys/mps.h \
	$(INCRT)/sys/alttbl.h \
	$(INCRT)/sys/fdisk.h \
	$(INCRT)/sys/ivlab.h \
	$(INCRT)/sys/vtoc.h \
	$(INCRT)/sys/bbh.h \
	$(INCRT)/sys/i258.h \
	$(FRC)
 
i258tp.o: i258tp.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/ics.h \
	$(INCRT)/sys/mps.h \
	$(INCRT)/sys/fdisk.h \
	$(INCRT)/sys/vtoc.h \
	$(INCRT)/sys/ivlab.h \
	$(INCRT)/sys/alttbl.h \
	$(INCRT)/sys/bbh.h \
	$(INCRT)/sys/i258.h \
	$(FRC)

clock.o: clock.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/rtc.h \
	$(INCRT)/sys/clockcal.h \
	$(FRC)

csmclk.o: csmclk.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/rtc.h \
	$(INCRT)/sys/clockcal.h \
	$(INCRT)/sys/ics.h \
	$(FRC)

i354.o: i354.c \
 	$(INCRT)/sys/signal.h \
 	$(INCRT)/sys/param.h \
 	$(INCRT)/sys/conf.h \
 	$(INCRT)/sys/fs/s5dir.h \
 	$(INCRT)/sys/user.h \
 	$(INCRT)/sys/tty.h \
 	$(INCRT)/sys/termio.h \
 	$(INCRT)/sys/file.h \
 	$(INCRT)/sys/sysinfo.h \
 	$(INCRT)/sys/i354.h \
	$(INCRT)/sys/i82530.h \
	$(FRC)

i350.o: lp.c \
	$(INCRT)/sys/i82530.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/iasy.h \
	$(INCRT)/sys/ddi.h \
	$(INCRT)/sys/lp_i350.h \
	$(FRC)
	cc $(CFLAGS) -DI350 -c lp.c
	mv lp.o i350.o


sysmsg.o: sysmsg.c \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/iasy.h \
	$(FRC)
