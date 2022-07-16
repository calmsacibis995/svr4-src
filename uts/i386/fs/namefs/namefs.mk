#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-fs:namefs/namefs.mk	1.3"

ROOT =
STRIP = strip
INCRT = ../..
INCFS = ..
OFILE = $(CONF)/pack.d/namefs/Driver.o

DASHG =
DASHO = -O
PFLAGS = -D_KERNEL $(MORECPP)
CFLAGS= $(DASHO) $(DASHG) $(PFLAGS) -I$(INCRT) -I$(INCFS)
DEFLIST =
FRC =

FILES =\
	namevno.o \
	namevfs.o

all:	$(OFILE)

$(OFILE): $(FILES)
	$(LD) -r -o $(OFILE) $(FILES)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(OFILE)

#
# Header dependencies
#


namevno.o: namevno.c \
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/cred.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/fcntl.h\
	$(INCRT)/sys/flock.h\
	$(INCRT)/sys/kmem.h\
	$(INCRT)/sys/uio.h\
	$(INCRT)/sys/vfs.h\
	$(INCRT)/sys/vnode.h\
	$(INCRT)/sys/immu.h\
	$(INCRT)/sys/tss.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/conf.h\
	$(INCRT)/vm/seg.h\
	$(INCRT)/sys/fs/namenode.h\
	$(INCRT)/sys/stream.h\
	$(INCRT)/fs/fs_subr.h \
	$(FRC)

namevfs.o: namevfs.c \
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/debug.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/kmem.h\
	$(INCRT)/sys/immu.h\
	$(INCRT)/sys/inline.h\
	$(INCRT)/sys/file.h\
	$(INCRT)/sys/proc.h\
	$(INCRT)/sys/stat.h\
	$(INCRT)/sys/statvfs.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/var.h\
	$(INCRT)/sys/vfs.h\
	$(INCRT)/sys/vnode.h\
	$(INCRT)/sys/mode.h\
	$(INCRT)/sys/tss.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/uio.h\
	$(INCRT)/sys/cred.h\
	$(INCRT)/sys/fs/namenode.h\
	$(INCRT)/sys/stream.h\
	$(INCRT)/sys/strsubr.h\
	$(INCRT)/sys/cmn_err.h\
	$(INCRT)/fs/fs_subr.h \
	$(FRC)



