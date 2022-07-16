#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-fs:fifofs/fifofs.mk	1.3"

STRIP = strip
INCRT = ../..
INCFS = ..
OFILE = $(CONF)/pack.d/fifofs/Driver.o

DASHG =
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS) -I$(INCRT) -I$(INCFS)
DEFLIST =
FRC =

FILES = \
	fifovnops.o \
	fifosubr.o 

all:	$(OFILE)

$(OFILE):	$(FILES)
	$(LD) -r -o $(OFILE) $(FILES)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(OFILE)

#
# Header dependencies
#

fifosubr.o: fifosubr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fs/fifonode.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/fs/fs_subr.h \
	$(FRC)

fifovnops.o: fifovnops.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fs/fifonode.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/fs/fs_subr.h \
	$(FRC)
