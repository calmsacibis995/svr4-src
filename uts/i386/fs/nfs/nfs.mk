#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-fs:nfs/nfs.mk	1.3.1.1"
#
#
#  		PROPRIETARY NOTICE (Combined)
#  
#  This source code is unpublished proprietary information
#  constituting, or derived under license from AT&T's Unix(r) System V.
#  
#  
#  
#  		Copyright Notice 
#  
#  Notice of copyright on this source code product does not indicate 
#  publication.
#  
#  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
#  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#  	          All rights reserved.
#
STRIP = strip
INCRT =  ../..
OFILE = $(CONF)/pack.d/nfs/Driver.o

DASHG =
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL -DSYSV $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS) -I$(INCRT)
DEFLIST =
FRC =

FILES = \
	nfs_aux.o\
	nfs_client.o\
	nfs_cnvt.o\
	nfs_common.o\
	nfs_export.o\
	nfs_server.o\
	nfs_subr.o\
	nfs_vfsops.o\
	nfs_vnops.o\
	nfs_xdr.o\
	nfssys.o

#	nfs_dump.o\

all:	$(OFILE)

$(OFILE): $(FILES)
	[ -d $(CONF)/pack.d/nfs ] || mkdir $(CONF)/pack.d/nfs; \
	$(LD) -r -o $(OFILE) $(FILES)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(OFILE)

#
# Header dependencies
#

nfs_aux.o: nfs_aux.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vfs.h \
	$(FRC)

nfs_client.o: nfs_client.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/clnt.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/nfs/nfs.h \
	$(INCRT)/nfs/nfs_clnt.h \
	$(INCRT)/nfs/rnode.h \
	$(FRC)

nfs_cnvt.o: nfs_cnvt.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/nfs/nfs.h \
	$(FRC)

nfs_common.o: nfs_common.c \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/nfs/nfs.h \
	$(INCRT)/sys/mode.h \
	$(FRC)

nfs_export.o: nfs_export.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/auth_unix.h \
	$(INCRT)/rpc/auth_des.h \
	$(INCRT)/nfs/nfs.h \
	$(INCRT)/nfs/export.h

nfs_server.o: nfs_server.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/siginfo.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/statvfs.h \
	$(INCRT)/sys/t_kuser.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/auth_unix.h \
	$(INCRT)/rpc/auth_des.h \
	$(INCRT)/rpc/svc.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/nfs/nfs.h \
	$(INCRT)/nfs/export.h \
	$(INCRT)/sys/dirent.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_map.h \
	$(INCRT)/vm/seg_kmem.h \
	$(FRC)

nfs_subr.o: nfs_subr.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/clnt.h \
	$(INCRT)/nfs/nfs.h \
	$(INCRT)/nfs/nfs_clnt.h \
	$(INCRT)/nfs/rnode.h \
	$(INCRT)/vm/pvn.h \
	$(FRC)

nfs_vfsops.o: nfs_vfsops.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/clnt.h \
	$(INCRT)/nfs/nfs.h \
	$(INCRT)/nfs/nfs_clnt.h \
	$(INCRT)/nfs/rnode.h \
	$(INCRT)/nfs/mount.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/statvfs.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

nfs_vnops.o: nfs_vnops.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/dirent.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/vmmeter.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/clnt.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/nfs/nfs.h \
	$(INCRT)/nfs/nfs_clnt.h \
	$(INCRT)/nfs/rnode.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_map.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/vm/rm.h \
	$(INCRT)/fs/fs_subr.h \
	$(INCRT)/klm/lockmgr.h \
	$(FRC)

nfs_xdr.o: nfs_xdr.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/dirent.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/nfs/nfs.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_map.h \
	$(INCRT)/vm/seg_kmem.h \
	$(FRC)

nfssys.o: nfssys.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/nfs/nfs.h \
	$(INCRT)/nfs/export.h \
	$(INCRT)/nfs/nfssys.h \
	$(FRC)
