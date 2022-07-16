#ident	"@(#)Makefile	1.2	91/09/22	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)xx:io/Makefile	1.2.1.2"

INC = $(ROOT)/usr/include
CC=cc
OFILE = Driver.o

DASHG =
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS) -I$(INC) -I..
DEFLIST =
FRC =

FILES = \
	xxalloc.o \
	xxblklist.o \
	xxbmap.o \
	xxdir.o \
	xxgetsz.o \
	xxinode.o \
	xxrdwri.o \
	xxvfsops.o \
	xxvnops.o

all:	$(OFILE)

$(OFILE):	$(FILES)
	$(LD) -r -o $(OFILE) $(FILES)

install:	all
	cp $(OFILE) ../pkg

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(OFILE)

FRC:

#
# Header dependencies
#

xxalloc.o: xxalloc.c \
	$(INC)/sys/types.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/cred.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/fcntl.h \
	$(INC)/sys/file.h \
	$(INC)/sys/flock.h \
	$(INC)/sys/param.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/swap.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/var.h \
	$(INC)/sys/vfs.h \
	$(INC)/sys/vnode.h \
	$(INC)/sys/mode.h \
	$(INC)/sys/user.h \
	$(INC)/vm/pvn.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/disp.h \
	$(INC)/sys/fs/s5param.h \
	../sys/fs/xxfblk.h \
	../sys/fs/xxfilsys.h \
	$(INC)/sys/fs/s5ino.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/fs/fs_subr.h \
	$(FRC)

s5blklist.o: s5blklist.c \
	$(INC)/sys/types.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/cred.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/fcntl.h \
	$(INC)/sys/file.h \
	$(INC)/sys/flock.h \
	$(INC)/sys/param.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/var.h \
	$(INC)/sys/vfs.h \
	$(INC)/sys/vnode.h \
	$(INC)/sys/mode.h \
	$(INC)/sys/user.h \
	$(INC)/vm/pvn.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/disp.h \
	$(INC)/sys/fs/s5param.h \
	../sys/fs/xxfblk.h \
	../sys/fs/xxfilsys.h \
	$(INC)/sys/fs/s5ino.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/fs/fs_subr.h \
	$(FRC)

xxbmap.o: xxbmap.c \
	$(INC)/sys/types.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/fbuf.h \
	$(INC)/sys/file.h \
	$(INC)/sys/param.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/vfs.h \
	$(INC)/sys/vnode.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/disp.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/vm/seg.h \
	$(FRC)

xxdir.o: xxdir.c \
	$(INC)/sys/types.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/cred.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/fbuf.h \
	$(INC)/sys/file.h \
	$(INC)/sys/param.h \
	$(INC)/sys/pathname.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/uio.h \
	$(INC)/sys/vfs.h \
	$(INC)/sys/vnode.h \
	$(INC)/sys/mode.h \
	$(INC)/sys/dnlc.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/disp.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/vm/seg.h \
	$(FRC)

xxgetsz.o: xxgetsz.c \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/vnode.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/disp.h \
	$(INC)/sys/fs/s5param.h \
	../sys/fs/xxfblk.h \
	../sys/fs/xxfilsys.h \
	$(INC)/sys/fs/s5ino.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/fs/s5macros.h \
	$(FRC)
	$(CC) -I$(INC) -O $(CFLAGS) $(PFLAGS) -c $<

xxinode.o: xxinode.c \
	$(INC)/sys/types.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/cred.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/file.h \
	$(INC)/sys/kmem.h \
	$(INC)/sys/open.h \
	$(INC)/sys/param.h \
	$(INC)/sys/time.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/swap.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/var.h \
	$(INC)/sys/vfs.h \
	$(INC)/sys/vnode.h \
	$(INC)/sys/mode.h \
	$(INC)/sys/dnlc.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/disp.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/sys/fs/s5ino.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/vm/page.h \
	$(INC)/vm/pvn.h \
	$(INC)/fs/fs_subr.h \
	$(FRC)

xxrdwri.o: xxrdwri.c \
	$(INC)/sys/types.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/file.h \
	$(INC)/sys/param.h \
	$(INC)/sys/swap.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/uio.h \
	$(INC)/sys/vfs.h \
	$(INC)/sys/vnode.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/disp.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/vm/seg_kmem.h \
	$(INC)/vm/seg_map.h \
	$(INC)/vm/seg.h \
	$(INC)/vm/page.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/kmem.h \
	$(FRC)

xxvfsops.o: xxvfsops.c \
	$(INC)/sys/types.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/fbuf.h \
	$(INC)/sys/file.h \
	$(INC)/sys/kmem.h \
	$(INC)/sys/mount.h \
	$(INC)/sys/open.h \
	$(INC)/sys/param.h \
	$(INC)/sys/time.h \
	$(INC)/sys/statvfs.h \
	$(INC)/sys/swap.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/uio.h \
	$(INC)/sys/var.h \
	$(INC)/sys/vfs.h \
	$(INC)/sys/vnode.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/tss.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/user.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/disp.h \
	$(INC)/vm/seg.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/sys/fs/s5inode.h \
	../sys/fs/xxfilsys.h \
	$(INC)/fs/fs_subr.h \
	$(FRC)

xxvnops.o: xxvnops.c \
	$(INC)/sys/types.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/cred.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/dirent.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/fbuf.h \
	$(INC)/sys/fcntl.h \
	$(INC)/sys/file.h \
	$(INC)/sys/flock.h \
	$(INC)/sys/kmem.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/inline.h \
	$(INC)/sys/mman.h \
	$(INC)/sys/open.h \
	$(INC)/sys/param.h \
	$(INC)/sys/time.h \
	$(INC)/sys/pathname.h \
	$(INC)/sys/sysinfo.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/uio.h \
	$(INC)/sys/var.h \
	$(INC)/sys/vfs.h \
	$(INC)/sys/vnode.h \
	$(INC)/sys/proc.h \
	$(INC)/sys/disp.h \
	$(INC)/sys/user.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fs/s5dir.h \
	../sys/fs/xxfilsys.h \
	$(INC)/sys/fs/s5inode.h \
	$(INC)/sys/fs/s5macros.h \
	$(INC)/vm/page.h \
	$(INC)/vm/pvn.h \
	$(INC)/vm/as.h \
	$(INC)/vm/seg.h \
	$(INC)/vm/seg_map.h \
	$(INC)/vm/seg_vn.h \
	$(INC)/vm/rm.h \
	$(INC)/fs/fs_subr.h \
	$(FRC)

