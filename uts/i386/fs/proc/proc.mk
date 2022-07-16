#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-fs:proc/proc.mk	1.3.1.1"

STRIP = strip
INCRT = ../..
INCFS = ..
CC = cc
OFILE = $(CONF)/pack.d/proc/Driver.o

DASHG = 
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS) -I$(INCRT) -I$(INCFS)
DEFLIST =
FRC =

FILES = prioctl.o prmachdep.o prsubr.o prusrio.o prvfsops.o prvnops.o
all:	$(OFILE)

$(OFILE): $(FILES)
	$(LD) -r -o $(OFILE) $(FILES)

lint:
	lint -u -x $(DEFLIST) -I$(INC) $(CFLAGS) \
		prioctl.c prmachdep.c prsubr.c prusrio.c prvfsops.c prvnops.c

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(OFILE)

#
# Header dependencies
#

prioctl.o: prioctl.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fault.h \
	$(INCRT)/sys/syscall.h \
	$(INCRT)/sys/procfs.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	prdata.h \
	$(FRC)

prmachdep.o: prmachdep.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fault.h \
	$(INCRT)/sys/syscall.h \
	$(INCRT)/sys/procfs.h \
	$(INCRT)/sys/debugreg.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/page.h \
	prdata.h \
	$(FRC)

prsubr.o: prsubr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/session.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/class.h \
	$(INCRT)/sys/ts.h \
	$(INCRT)/sys/bitmap.h \
	$(INCRT)/sys/fault.h \
	$(INCRT)/sys/syscall.h \
	$(INCRT)/sys/procfs.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/rm.h \
	$(INCRT)/vm/seg.h \
	prdata.h \
	$(FRC)

prusrio.o: prusrio.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/procfs.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	prdata.h \
	$(FRC)

prvfsops.o: prvfsops.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/procfs.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/statvfs.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/mode.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/fs/fs_subr.h \
	prdata.h \
	$(FRC)

prvnops.o: prvnops.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/dirent.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/mode.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fault.h \
	$(INCRT)/sys/syscall.h \
	$(INCRT)/sys/procfs.h \
	$(INCRT)/fs/fs_subr.h \
	$(INCRT)/vm/rm.h \
	prdata.h \
	$(FRC)

