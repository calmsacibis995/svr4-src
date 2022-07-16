#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-fs:rfs/rfs.mk	1.3.1.1"

ROOT =
STRIP = strip
INCRT = ../..
OFILE = $(CONF)/pack.d/RFS/Driver.o
DASHG =
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL $(MORECPP) 
CFLAGS= $(DASHO) $(PFLAGS) -I$(INCRT)
DEFLIST=
FRC =

LINT = $(PFX)lint
LINTFLAGS = -unx

MAKEFILE= rfs.mk
MAKEFLAGS =

O = o

FILES = \
	rf_auth.$O \
	rf_cirmgr.$O \
	rf_comm.$O \
	rf_rsrc.$O \
	rf_admin.$O \
	rf_canon.$O \
	rf_sys.$O \
	rfsr_subr.$O \
	rfsr_ops.$O \
	rf_serve.$O \
	du.$O \
	rf_vfsops.$O \
	rf_getsz.$O \
	rf_cache.$O \
	rfcl_subr.$O \
	rf_vnops.$O

.SUFFIXES: .o .ln

all:	$(OFILE)

lint:
	$(MAKE) -ef $(MAKEFILE) -$(MAKEFLAGS) CC=$(LINT) INC=$(INC) \
		MORECPP=-UDEBUG O=ln lintit

lintit: $(FILES)
	$(LINT) $(LINTFLAGS) $(FILES)

clean:
	-rm -f *.o  `ls *.ln | egrep -v llib` *.ln

clobber:	clean
	-rm -f $(OFILE) llib*

$(OFILE):	$(FILES)
	[ -d $(CONF)/pack.d/RFS ] || mkdir $(CONF)/pack.d/RFS; \
	$(LD) -r -o $(OFILE) $(FILES)

FRC:

#
# Header dependencies
#

rf_auth.$O: \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/rf_cirmgr.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/idtab.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rf_debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/systm.h \
	rf_auth.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/rf_sys.h \
	$(INCRT)/sys/list.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/rf_comm.h \
	rf_canon.h \
	$(FRC)

rf_cirmgr.$O: \
	$(INCRT)/sys/list.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/sys/rf_comm.h \
	$(INCRT)/sys/inline.h \
	rf_admin.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/rf_cirmgr.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/hetero.h \
	$(INCRT)/sys/systm.h \
	rf_auth.h \
	$(INCRT)/sys/cmn_err.h \
	rf_canon.h \
	$(INCRT)/sys/strmdep.h \
	$(FRC)

rf_comm.$O: \
	$(INCRT)/sys/list.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/sys/rf_comm.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/rf_cirmgr.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rf_debug.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/hetero.h \
	rf_canon.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/statfs.h \
	rfcl_subr.h \
	$(INCRT)/sys/rf_adv.h \
	rf_serve.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/fs/rf_acct.h \
	$(INCRT)/sys/fs/rf_vfs.h \
	$(INCRT)/sys/file.h \
	rf_admin.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/vm/page.h \
	rf_cache.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/fbuf.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/pvn.h \
	$(FRC)

rf_rsrc.$O: \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/list.h \
	$(INCRT)/sys/rf_adv.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/rf_sys.h \
	$(FRC)

rf_admin.$O: \
	$(INCRT)/sys/list.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/fs/rf_acct.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/rf_comm.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/wait.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/rf_cirmgr.h \
	$(INCRT)/sys/rf_debug.h \
	$(INCRT)/sys/rf_sys.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/rf_adv.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/fs/rf_vfs.h \
	rf_serve.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/statfs.h \
	rfcl_subr.h \
	rf_admin.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/vm/page.h \
	rf_cache.h \
	$(INCRT)/vm/seg_kmem.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/session.h \
	$(INCRT)/sys/kmem.h \
	$(FRC)

rf_canon.$O: \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/rf_cirmgr.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/dirent.h \
	rf_canon.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/errno.h \
	$(FRC)

rf_sys.$O: \
	$(INCRT)/sys/list.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/rf_cirmgr.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/sys/rf_comm.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/rf_debug.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rf_adv.h \
	$(INCRT)/sys/rf_sys.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/idtab.h \
	$(INCRT)/sys/hetero.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/rf_vfs.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/statvfs.h \
	$(INCRT)/sys/statfs.h \
	rf_auth.h \
	$(INCRT)/sys/dirent.h \
	rf_admin.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/vm/page.h \
	rf_cache.h \
	$(INCRT)/sys/file.h \
	rfcl_subr.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/kmem.h \
	rf_serve.h \
	$(FRC)

rfsr_subr.$O: \
	$(INCRT)/sys/list.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/mode.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/vm/seg.h \
	rf_admin.h \
	$(INCRT)/sys/rf_comm.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/siginfo.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/rf_cirmgr.h \
	$(INCRT)/sys/idtab.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/rf_adv.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/fs/rf_vfs.h \
	$(INCRT)/sys/rf_messg.h \
	rf_serve.h \
	$(INCRT)/sys/dirent.h \
	$(INCRT)/sys/statfs.h \
	$(INCRT)/sys/statvfs.h \
	rf_auth.h \
	rfcl_subr.h \
	$(INCRT)/vm/page.h \
	rf_cache.h \
	$(INCRT)/sys/hetero.h \
	rf_canon.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/fs/rf_acct.h \
	$(FRC)

rfsr_ops.$O: \
	$(INCRT)/sys/list.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/fs/rf_acct.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/mode.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/sys/rf_comm.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/dirent.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/rf_cirmgr.h \
	$(INCRT)/sys/idtab.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/statfs.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rf_debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/rf_adv.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/fs/rf_vfs.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/hetero.h \
	rf_serve.h \
	$(INCRT)/sys/ustat.h \
	$(INCRT)/sys/statvfs.h \
	rfcl_subr.h \
	rf_auth.h \
	$(INCRT)/sys/fbuf.h \
	$(INCRT)/vm/seg_map.h \
	rf_canon.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/vm/page.h \
	rf_cache.h \
	du.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/fs/fs_subr.h \
	$(INCRT)/sys/stropts.h \
	$(FRC)

rf_serve.$O: \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/rf_cirmgr.h \
	$(INCRT)/sys/list.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/sys/rf_comm.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/idtab.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/mode.h \
	$(INCRT)/sys/rf_adv.h \
	rf_serve.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/vm/seg_kmem.h \
	rf_auth.h \
	rf_admin.h \
	$(FRC)

du.$O: \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/fs/rf_acct.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/stream.h \
	rf_admin.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/list.h \
	$(INCRT)/sys/mode.h \
	$(INCRT)/sys/idtab.h \
	$(INCRT)/sys/rf_cirmgr.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/sys/rf_comm.h \
	$(INCRT)/sys/fs/rf_vfs.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rf_debug.h \
	$(INCRT)/sys/rf_adv.h \
	rf_serve.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/utime.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/statfs.h \
	$(INCRT)/sys/statvfs.h \
	$(INCRT)/sys/hetero.h \
	rf_canon.h \
	rfcl_subr.h \
	rf_auth.h \
	du.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/vm/page.h \
	rf_cache.h \
	$(FRC)

rf_vfsops.$O: \
	$(INCRT)/sys/list.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/fs/rf_acct.h \
	$(INCRT)/sys/bitmap.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/sys/rf_comm.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/rf_debug.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/rf_cirmgr.h \
	$(INCRT)/sys/fs/rf_vfs.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/statfs.h \
	$(INCRT)/sys/statvfs.h \
	$(INCRT)/sys/ustat.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/rf_adv.h \
	rfcl_subr.h \
	du.h \
	rf_admin.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/mode.h \
	$(INCRT)/sys/hetero.h \
	rf_canon.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/vm/page.h \
	rf_cache.h \
	$(INCRT)/sys/kmem.h \
	$(FRC)

rf_getsz.$O: \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/rf_adv.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/statfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/list.h \
	$(INCRT)/sys/rf_cirmgr.h \
	$(INCRT)/sys/rf_debug.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/sys/rf_comm.h \
	rf_admin.h \
	rf_serve.h \
	$(INCRT)/sys/fs/rf_vfs.h \
	rfcl_subr.h \
	du.h \
	$(FRC)
	$(CC) $(PFLAGS) -I$(INCRT) -c -g $<

rf_cache.$O: \
	$(INCRT)/sys/list.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/fs/rf_acct.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/rf_cirmgr.h \
	$(INCRT)/sys/hetero.h \
	$(INCRT)/vm/seg_map.h \
	$(INCRT)/sys/rf_comm.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/vm/rm.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/statfs.h \
	$(INCRT)/sys/buf.h \
	rf_cache.h \
	rfcl_subr.h \
	rf_admin.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/fs/rf_vfs.h \
	$(FRC)

rfcl_subr.$O: \
	$(INCRT)/sys/list.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/rf_cirmgr.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fs/rf_vfs.h \
	$(INCRT)/sys/fs/rf_acct.h \
	$(INCRT)/sys/rf_sys.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/sys/rf_comm.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/hetero.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/statfs.h \
	rfcl_subr.h \
	rf_admin.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/mode.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/seg_map.h \
	$(INCRT)/vm/pvn.h \
	rf_cache.h \
	$(FRC)

rf_vnops.$O: \
	$(INCRT)/sys/list.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/fs/rf_acct.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/statvfs.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/sys/rf_comm.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/rf_cirmgr.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/statfs.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/mode.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	rf_admin.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/vm/rm.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/sys/fs/rf_vfs.h \
	$(INCRT)/fs/fs_subr.h \
	$(INCRT)/sys/hetero.h \
	rf_canon.h \
	rfcl_subr.h \
	du.h \
	rf_cache.h \
	rf_auth.h \
	$(INCRT)/sys/idtab.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/vm/seg_map.h \
	$(FRC)
