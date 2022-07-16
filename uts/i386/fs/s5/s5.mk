#ident	"@(#)s5.mk	1.2	91/09/22	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-fs:s5/s5.mk	1.2.1.1"

STRIP = strip
INCRT = ../..
CC=cc
OFILE = $(CONF)/pack.d/s5/Driver.o

DASHG =
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL -Di386 $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS) -I$(INCRT) 
DEFLIST =
FRC =

FILES = \
	s5alloc.o \
	s5blklist.o \
	s5bmap.o \
	s5dir.o \
	s5getsz.o \
	s5inode.o \
	s5rdwri.o \
	s5vfsops.o \
	s5vnops.o

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

s5alloc.o: s5alloc.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/mode.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5fblk.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/sys/fs/s5ino.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/fs/fs_subr.h \
	$(FRC)

s5blklist.o: s5blklist.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/mode.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5fblk.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/sys/fs/s5ino.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/fs/fs_subr.h \
	$(FRC)

s5bmap.o: s5bmap.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fbuf.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/vm/seg.h \
	$(FRC)

s5dir.o: s5dir.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fbuf.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/mode.h \
	$(INCRT)/sys/dnlc.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/vm/seg.h \
	$(FRC)

s5getsz.o: s5getsz.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5fblk.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/sys/fs/s5ino.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(FRC)
	$(CC) -I$(INCRT) -O $(PFLAGS) -c $<

s5inode.o: s5inode.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/mode.h \
	$(INCRT)/sys/dnlc.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/fs/s5ino.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/fs/fs_subr.h \
	$(FRC)

s5rdwri.o: s5rdwri.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/vm/seg_kmem.h \
	$(INCRT)/vm/seg_map.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/kmem.h \
	$(FRC)

s5vfsops.o: s5vfsops.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fbuf.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/statvfs.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/fs/fs_subr.h \
	$(FRC)

s5vnops.o: s5vnops.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/dirent.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fbuf.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_map.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/vm/rm.h \
	$(INCRT)/fs/fs_subr.h \
	$(FRC)

