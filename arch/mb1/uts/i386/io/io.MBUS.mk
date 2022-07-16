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


#ident	"@(#)mb1:uts/i386/io/io.MBUS.mk	1.3.1.1"

INCRT = ..
CFLAGS = -O -I$(INCRT) -D_KERNEL -DSYSV $(MORECPP)
FRC =
BUS=

FILES =\
	$(CONF)/pack.d/i214/Driver.o	\
	$(CONF)/pack.d/i214tp/Driver.o	\
	$(CONF)/pack.d/i546/Driver.o	\
	$(CONF)/pack.d/i8251/Driver.o   \
	$(CONF)/pack.d/dma/Driver.o	\
	$(CONF)/pack.d/iasy/Driver.o	\
	$(CONF)/pack.d/console/Driver.o   \
	$(CONF)/pack.d/sysmsg/Driver.o   \
	$(CONF)/pack.d/clock/Driver.o   \
	$(CONF)/pack.d/lma/Driver.o   \
	$(CONF)/pack.d/bps/Driver.o   \
	$(CONF)/pack.d/ramd/Driver.o

all:	
	$(MAKE) -f io.$(ARCH).mk $(FILES)

clean:

clobber:	clean
	-rm -f $(MB1FILES)

FRC:

#
# MB1 Drivers
#

$(CONF)/pack.d/i214/Driver.o:	i214.o 	
	$(LD) -r -o $@ i214.o 

$(CONF)/pack.d/ramd/Driver.o:    ramd.o 
	$(LD) -r -o $@ ramd.o 

$(CONF)/pack.d/dma/Driver.o:    dma.o 
	$(LD) -r -o $@ dma.o 

$(CONF)/pack.d/i214tp/Driver.o:i214tp.o
	$(LD) -r -o $@ i214tp.o

$(CONF)/pack.d/iasy/Driver.o:	iasy.o
	$(LD) -r -o $@ iasy.o

$(CONF)/pack.d/i546/Driver.o:	i546.o 
	$(LD) -r -o $@ i546.o

$(CONF)/pack.d/i8251/Driver.o:	i8251.o 
	$(LD) -r -o $@ i8251.o 

$(CONF)/pack.d/console/Driver.o:	console.o
	$(LD) -r -o $@ console.o

$(CONF)/pack.d/clock/Driver.o:	clock.o
	$(LD) -r -o $@ clock.o

$(CONF)/pack.d/lma/Driver.o:	lma.o
	$(LD) -r -o $@ lma.o

$(CONF)/pack.d/sysmsg/Driver.o:	sysmsg.o
	$(LD) -r -o $@ sysmsg.o

$(CONF)/pack.d/bps/Driver.o:	bps.o
	[ -d $(CONF)/pack.d/bps ] || mkdir $(CONF)/pack.d/bps; \
	$(LD) -r -o $@ bps.o

#
# Header dependencies
#

i214.o: i214.c \
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
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/alttbl.h \
	$(INCRT)/sys/fdisk.h \
	$(INCRT)/sys/ivlab.h \
	$(INCRT)/sys/vtoc.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/bbh.h \
	$(INCRT)/sys/i214.h \
	$(FRC)


i214tp.o: i214tp.c \
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
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/fdisk.h \
	$(INCRT)/sys/ivlab.h \
	$(INCRT)/sys/vtoc.h \
	$(INCRT)/sys/alttbl.h \
	$(INCRT)/sys/bbh.h \
	$(INCRT)/sys/i214.h \
	$(FRC)

i546.o: i546.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/clockcal.h \
	$(INCRT)/sys/i546.h \
	$(FRC)

i8251.o: i8251.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/i8251.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

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

clock.o: clock.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/rtc.h \
	$(INCRT)/sys/clockcal.h \
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
	$(INCRT)/sys/ccimp.h \
	$(INCRT)/sys/iasy.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/ddi.h \
	$(FRC)
 
sysmsg.o: sysmsg.c \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/iasy.h \
	$(FRC)

