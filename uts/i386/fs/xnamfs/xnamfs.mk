#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-fs:xnamfs/xnamfs.mk	1.3.1.1"

STRIP = strip
INCRT = ../..
OFILE = $(CONF)/pack.d/xnamfs/Driver.o

DASHG =
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS) -I$(INCRT)
DEFLIST =
FRC =

FILES = \
	xnamsubr.o \
	xnamvfsops.o \
	xnamvnops.o \
	xsd.o \
	xsem.o

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

xnamsubr.o: xnamsubr.c \
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
	$(INCRT)/sys/fs/xnamnode.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/file.h \
	$(FRC)

xnamvfsops.o: xnamvfsops.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/fs/xnamnode.h \
	$(INCRT)/fs/fs_subr.h \
	$(FRC)

xnamvnops.o: xnamvnops.c \
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
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/poll.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fs/xnamnode.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_map.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/vm/seg_dev.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/fs/fs_subr.h \
	$(FRC)

xsem.o: xsem.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/fs/xnamnode.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/sysinfo.h \
	$(FRC)

xsd.o: xsd.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/vm.h \
	$(INCRT)/sys/sd.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/fs/xnamnode.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

