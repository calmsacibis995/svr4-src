#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-fs:specfs/specfs.mk	1.3.1.1"

STRIP = strip
INCRT = ../..
INCFS = ..
CC = cc
OFILE = $(CONF)/pack.d/specfs/Driver.o

DASHG =
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS) -I$(INCRT) -I$(INCFS)
DEFLIST =
FRC =

FILES = \
	specgetsz.o \
	specsubr.o \
	specvfsops.o \
	specvnops.o

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

specgetsz.o: specgetsz.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/fs/snode.h \
	$(FRC)
	$(CC) $(PFLAGS) -I$(INCRT) -c -g $<

specsubr.o: specsubr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/fs/snode.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strsubr.h \
	$(FRC)

specvfsops.o: specvfsops.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/fs/snode.h \
	$(INCRT)/fs/fs_subr.h \
	$(FRC)

specvnops.o: specvnops.c \
	$(INCRT)/sys/session.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/poll.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fs/snode.h \
	$(INCRT)/sys/vmparam.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_map.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/vm/seg_dev.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/fs/fs_subr.h \
	$(FRC)

