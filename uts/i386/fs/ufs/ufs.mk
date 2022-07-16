#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-fs:ufs/ufs.mk	1.3"

STRIP = strip
INCRT = ../..
OFILE = $(CONF)/pack.d/ufs/Driver.o

DASHG =
DASHO = -O
PFLAGS = $(DASHG) -DQUOTA -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS) -I$(INCRT) 
DEFLIST =
FRC =

FILES = \
	quota.o \
	quotacalls.o \
	quota_ufs.o \
	ufs_alloc.o \
	ufs_bmap.o \
	ufs_blklist.o \
	ufs_dir.o \
	ufs_dsort.o \
	ufs_inode.o \
	ufs_subr.o \
	ufs_tables.o \
	ufs_vfsops.o \
	ufs_vnops.o

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

quota.o: quota.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/errno.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/uio.h \
        $(INCRT)/sys/fs/ufs_quota.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/cmn_err.h \
        $(INCRT)/sys/kmem.h
	${CC} -c ${CFLAGS} quota.c

quota.L: quota.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/errno.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/uio.h \
        $(INCRT)/sys/fs/ufs_quota.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/cmn_err.h \
        $(INCRT)/sys/kmem.h
	@echo quota.c:
	@-(${CPP} ${LCOPTS} quota.c | \
	  ${LINT1} ${LOPTS} > quota.L ) 2>&1 | ${LTAIL}

quotacalls.o: quotacalls.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/cred.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/uio.h \
        $(INCRT)/sys/fs/ufs_quota.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/errno.h
	${CC} -c ${CFLAGS} quotacalls.c

quotacalls.L: quotacalls.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/cred.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/uio.h \
        $(INCRT)/sys/fs/ufs_quota.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/errno.h \
#       $(INCRT)/sys/quota.h
	@echo quotacalls.c:
	@-(${CPP} ${LCOPTS} quotacalls.c | \
	  ${LINT1} ${LOPTS} > quotacalls.L ) 2>&1 | ${LTAIL}

quota_ufs.o: quota_ufs.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/cred.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/buf.h \
        $(INCRT)/sys/uio.h \
        $(INCRT)/sys/fs/ufs_quota.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/errno.h
	${CC} -c ${CFLAGS} quota_ufs.c

quota_ufs.L: quota_ufs.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/cred.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/buf.h \
        $(INCRT)/sys/uio.h \
        $(INCRT)/sys/fs/ufs_quota.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/errno.h
	@echo quota_ufs.c:
	@-(${CPP} ${LCOPTS} quota_ufs.c | \
	  ${LINT1} ${LOPTS} > quota_ufs.L ) 2>&1 | ${LTAIL}

ufs_alloc.o: ufs_alloc.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/debug.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/cred.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/disp.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/buf.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/fs/ufs_quota.h \
        $(INCRT)/sys/errno.h \
        $(INCRT)/sys/time.h \
        $(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/fs/fs_subr.h \
	$(INCRT)/sys/cmn_err.h
	${CC} -c ${CFLAGS} ufs_alloc.c
 
ufs_alloc.L: ufs_alloc.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/debug.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/cred.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/disp.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/buf.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/fs/ufs_quota.h \
        $(INCRT)/sys/errno.h \
        $(INCRT)/sys/time.h \
        $(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/fs/fs_subr.h \
	$(INCRT)/sys/cmn_err.h
	@echo ufs_alloc.c:
	@-(${CPP} ${LCOPTS} ufs_alloc.c | \
	  ${LINT1} ${LOPTS} > ufs_alloc.L ) 2>&1 | ${LTAIL}
 
ufs_bmap.o: ufs_bmap.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/buf.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/conf.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/vm/seg.h \
        $(INCRT)/sys/errno.h \
        $(INCRT)/sys/sysmacros.h
	${CC} -c ${CFLAGS} ufs_bmap.c
 
ufs_bmap.L: ufs_bmap.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/buf.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/conf.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/vm/seg.h \
        $(INCRT)/sys/errno.h \
        $(INCRT)/sys/sysmacros.h
	@echo ufs_bmap.c:
	@-(${CPP} ${LCOPTS} ufs_bmap.c | \
	  ${LINT1} ${LOPTS} > ufs_bmap.L ) 2>&1 | ${LTAIL}
 
ufs_blklist.o: ufs_blklist.c \
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
	$(INCRT)/sys/fs/ufs_fs.h \
	$(INCRT)/sys/fs/ufs_inode.h \
	$(INCRT)/fs/fs_subr.h
	${CC} -c ${CFLAGS} ufs_blklist.c

ufs_dir.o: ufs_dir.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/cred.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/stat.h \
        $(INCRT)/sys/mode.h \
        $(INCRT)/sys/buf.h \
        $(INCRT)/sys/uio.h \
        $(INCRT)/sys/dnlc.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/mount.h \
        $(INCRT)/sys/fs/ufs_fsdir.h \
        $(INCRT)/sys/fs/ufs_quota.h \
        $(INCRT)/sys/errno.h \
        $(INCRT)/sys/debug.h \
        $(INCRT)/vm/seg.h \
        $(INCRT)/sys/sysmacros.h
	${CC} -c ${CFLAGS} ufs_dir.c
 
ufs_dir.L: ufs_dir.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/cred.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/stat.h \
        $(INCRT)/sys/mode.h \
        $(INCRT)/sys/buf.h \
        $(INCRT)/sys/uio.h \
        $(INCRT)/sys/dnlc.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/mount.h \
        $(INCRT)/sys/fs/ufs_fsdir.h \
        $(INCRT)/sys/fs/ufs_quota.h \
        $(INCRT)/sys/errno.h \
        $(INCRT)/sys/debug.h \
        $(INCRT)/vm/seg.h \
        $(INCRT)/sys/sysmacros.h
	@echo ufs_dir.c:
	@-(${CPP} ${LCOPTS} ufs_dir.c | \
	  ${LINT1} ${LOPTS} > ufs_dir.L ) 2>&1 | ${LTAIL}
 
ufs_dsort.o: ufs_dsort.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/buf.h
	${CC} -c ${CFLAGS} ufs_dsort.c
 
ufs_dsort.L: ufs_dsort.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/buf.h
	@echo ufs_dsort.c:
	@-(${CPP} ${LCOPTS} ufs_dsort.c | \
	  ${LINT1} ${LOPTS} > ufs_dsort.L ) 2>&1 | ${LTAIL}
 
ufs_inode.o: ufs_inode.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/cred.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/stat.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/buf.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/mode.h \
        $(INCRT)/sys/cmn_err.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/fs/ufs_quota.h \
        $(INCRT)/vm/vm_hat.h \
        $(INCRT)/vm/as.h \
        $(INCRT)/vm/pvn.h \
        $(INCRT)/vm/seg.h \
        $(INCRT)/sys/swap.h \
        $(INCRT)/sys/sysinfo.h \
        $(INCRT)/sys/sysmacros.h \
        $(INCRT)/sys/errno.h \
        $(INCRT)/sys/cmn_err.h \
        $(INCRT)/sys/kmem.h \
        $(INCRT)/sys/debug.h \
	$(INCRT)/fs/fs_subr.h
	${CC} -c ${CFLAGS} ufs_inode.c
 
ufs_inode.L: ufs_inode.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/cred.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/stat.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/buf.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/mode.h \
        $(INCRT)/sys/cmn_err.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/fs/ufs_quota.h \
        $(INCRT)/vm/vm_hat.h \
        $(INCRT)/vm/as.h \
        $(INCRT)/vm/pvn.h \
        $(INCRT)/vm/seg.h \
        $(INCRT)/sys/swap.h \
        $(INCRT)/sys/sysinfo.h \
        $(INCRT)/sys/sysmacros.h \
        $(INCRT)/sys/errno.h \
        $(INCRT)/sys/cmn_err.h \
        $(INCRT)/sys/kmem.h \
        $(INCRT)/sys/debug.h \
	$(INCRT)/fs/fs_subr.h
	@echo ufs_inode.c:
	@-(${CPP} ${LCOPTS} ufs_inode.c | \
	  ${LINT1} ${LOPTS} > ufs_inode.L ) 2>&1 | ${LTAIL}
 
ufs_subr.o: ufs_subr.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/time.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/sysmacros.h \
        $(INCRT)/sys/buf.h \
        $(INCRT)/sys/conf.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/debug.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/vm/vm_hat.h \
        $(INCRT)/vm/as.h \
        $(INCRT)/vm/seg.h \
        $(INCRT)/vm/page.h \
        $(INCRT)/vm/pvn.h \
        $(INCRT)/vm/seg_map.h \
        $(INCRT)/sys/swap.h \
        $(INCRT)/vm/seg_kmem.h
	${CC} -c ${CFLAGS} ufs_subr.c
 
ufs_subr.L: ufs_subr.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/time.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/sysmacros.h \
        $(INCRT)/sys/buf.h \
        $(INCRT)/sys/conf.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/debug.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/vm/vm_hat.h \
        $(INCRT)/vm/as.h \
        $(INCRT)/vm/seg.h \
        $(INCRT)/vm/page.h \
        $(INCRT)/vm/pvn.h \
        $(INCRT)/vm/seg_map.h \
        $(INCRT)/sys/swap.h \
        $(INCRT)/vm/seg_kmem.h
	@echo ufs_subr.c:
	@-(${CPP} ${LCOPTS} ufs_subr.c | \
	  ${LINT1} ${LOPTS} > ufs_subr.L ) 2>&1 | ${LTAIL}
 
ufs_tables.o: ufs_tables.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h
	${CC} -c ${CFLAGS} ufs_tables.c
 
ufs_tables.L: ufs_tables.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h
	@echo ufs_tables.c:
	@-(${CPP} ${LCOPTS} ufs_tables.c | \
	  ${LINT1} ${LOPTS} > ufs_tables.L ) 2>&1 | ${LTAIL}
 
ufs_vfsops.o: ufs_vfsops.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/sysmacros.h \
        $(INCRT)/sys/kmem.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/buf.h \
        $(INCRT)/sys/pathname.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/file.h \
        $(INCRT)/sys/uio.h \
        $(INCRT)/sys/conf.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/statvfs.h \
        $(INCRT)/sys/mount.h \
        $(INCRT)/sys/swap.h \
        $(INCRT)/sys/errno.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/fs/fs_subr.h
	${CC} -c ${CFLAGS} ufs_vfsops.c
 
ufs_vfsops.L: ufs_vfsops.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/sysmacros.h \
        $(INCRT)/sys/kmem.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/buf.h \
        $(INCRT)/sys/pathname.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/file.h \
        $(INCRT)/sys/uio.h \
        $(INCRT)/sys/conf.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/statvfs.h \
        $(INCRT)/sys/mount.h \
        $(INCRT)/sys/swap.h \
        $(INCRT)/sys/errno.h \
	$(INCRT)/fs/fs_subr.h
	@echo ufs_vfsops.c:
	@-(${CPP} ${LCOPTS} ufs_vfsops.c | \
	  ${LINT1} ${LOPTS} > ufs_vfsops.L ) 2>&1 | ${LTAIL}
 
ufs_vnops.o: ufs_vnops.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/time.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/sysmacros.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/cred.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/buf.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/file.h \
        $(INCRT)/sys/fcntl.h \
        $(INCRT)/sys/flock.h \
        $(INCRT)/sys/kmem.h \
        $(INCRT)/sys/uio.h \
        $(INCRT)/sys/conf.h \
        $(INCRT)/sys/mman.h \
        $(INCRT)/sys/pathname.h \
        $(INCRT)/sys/debug.h \
        $(INCRT)/sys/vmmeter.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/fs/ufs_fsdir.h \
        $(INCRT)/sys/fs/ufs_quota.h \
        $(INCRT)/sys/dirent.h \
        $(INCRT)/sys/errno.h \
        $(INCRT)/sys/sysinfo.h \
        $(INCRT)/sys/cmn_err.h \
        $(INCRT)/vm/vm_hat.h \
        $(INCRT)/vm/page.h \
        $(INCRT)/vm/pvn.h \
        $(INCRT)/vm/as.h \
        $(INCRT)/vm/seg.h \
        $(INCRT)/vm/seg_map.h \
        $(INCRT)/vm/seg_vn.h \
        $(INCRT)/vm/rm.h \
        $(INCRT)/sys/swap.h \
	$(INCRT)/fs/fs_subr.h
	${CC} -c ${CFLAGS} ufs_vnops.c
 
ufs_vnops.L: ufs_vnops.c \
        $(INCRT)/sys/types.h \
        $(INCRT)/sys/param.h \
        $(INCRT)/sys/time.h \
        $(INCRT)/sys/systm.h \
        $(INCRT)/sys/sysmacros.h \
        $(INCRT)/sys/signal.h \
        $(INCRT)/sys/cred.h \
        $(INCRT)/sys/user.h \
        $(INCRT)/sys/buf.h \
        $(INCRT)/sys/vfs.h \
        $(INCRT)/sys/vnode.h \
        $(INCRT)/sys/proc.h \
        $(INCRT)/sys/file.h \
        $(INCRT)/sys/fcntl.h \
        $(INCRT)/sys/flock.h \
        $(INCRT)/sys/kmem.h \
        $(INCRT)/sys/uio.h \
        $(INCRT)/sys/conf.h \
        $(INCRT)/sys/mman.h \
        $(INCRT)/sys/pathname.h \
        $(INCRT)/sys/debug.h \
        $(INCRT)/sys/vmmeter.h \
        $(INCRT)/sys/fs/ufs_fs.h \
        $(INCRT)/sys/fs/ufs_inode.h \
        $(INCRT)/sys/fs/ufs_fsdir.h \
        $(INCRT)/sys/fs/ufs_quota.h \
        $(INCRT)/sys/dirent.h \
        $(INCRT)/sys/errno.h \
        $(INCRT)/sys/sysinfo.h \
        $(INCRT)/sys/cmn_err.h \
        $(INCRT)/vm/vm_hat.h \
        $(INCRT)/vm/page.h \
        $(INCRT)/vm/pvn.h \
        $(INCRT)/vm/as.h \
        $(INCRT)/vm/seg.h \
        $(INCRT)/vm/seg_map.h \
        $(INCRT)/vm/seg_vn.h \
        $(INCRT)/vm/rm.h \
        $(INCRT)/sys/swap.h \
	$(INCRT)/fs/fs_subr.h
	@echo ufs_vnops.c:
	@-(${CPP} ${LCOPTS} ufs_vnops.c | \
	  ${LINT1} ${LOPTS} > ufs_vnops.L ) 2>&1 | ${LTAIL}
 
