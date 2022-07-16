#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-fs:bfs/bfs.mk	1.3"

STRIP = strip
INCRT = ../..
CC=cc
OFILE = $(CONF)/pack.d/bfs/Driver.o

DASHG =
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS) -I$(INCRT) 
DEFLIST =
FRC =

FILES =\
	bfs_compact.o \
	bfs_subr.o \
	bfs_vfsops.o \
	bfs_vnops.o

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

bfs_compact.o: bfs_compact.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fs/bfs.h \
	$(INCRT)/sys/fs/bfs_compact.h \
	$(FRC)

bfs_subr.o: bfs_subr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fs/bfs.h \
	$(INCRT)/sys/flock.h \
	$(FRC)

bfs_vfsops.o: bfs_vfsops.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/statvfs.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fs/bfs.h \
	$(INCRT)/fs/fs_subr.h \
	$(FRC)

bfs_vnops.o: bfs_vnops.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/dirent.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/fs/bfs.h \
	$(INCRT)/fs/fs_subr.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/disp.h \
	$(FRC)

