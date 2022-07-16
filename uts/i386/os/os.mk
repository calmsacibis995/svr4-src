#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.

#ident	"@(#)kern-os:os.mk	1.3.3.2"

INCRT = ..
CFLAGS = -I$(INCRT) -D_KERNEL -O $(MORECPP) 
OFILE = $(CONF)/pack.d/kernel/os.o
OFP = $(CONF)/pack.d/fp/Driver.o
ONMI = $(CONF)/pack.d/nmi/Driver.o
OIPC = $(CONF)/pack.d/ipc/Driver.o
OMSG = $(CONF)/pack.d/msg/Driver.o
OSEM = $(CONF)/pack.d/sem/Driver.o
OSHM = $(CONF)/pack.d/shm/Driver.o
OKMA = $(CONF)/pack.d/kma/Driver.o

FRC =

FILES =\
	acct.o \
	bio.o \
	bitmasks.o \
	bitmap.o \
	bs.o \
	clock.o \
	cmn_err.o \
	core.o \
	cred.o \
	cxenix.o \
	ddi.o \
	exec.o \
	execargs.o \
	exit.o \
	fbio.o \
	fio.o \
	flock.o \
	fork.o \
	grow.o \
	kperf.o \
	list.o \
	local.o \
	lock.o \
	machdep.o \
	main.o \
	malloc.o \
	mapfile.o \
	mod.o \
	move.o \
	name.o \
	oem.o\
	pgrp.o \
	pid.o \
	pipe.o \
	pmstub.o \
	predki.o \
	procset.o \
	scalls.o \
	sched.o \
	session.o \
	sig.o \
	slp.o \
	startup.o \
	streamio.o \
	strsubr.o \
	subr.o \
	sysent.o \
	sysi86.o \
	todc.o \
	trap.o \
	vm_meter.o \
	vm_pageout.o \
	vm_subr.o \
	xmmu.o \
	xsys.o

all:    $(OFP) $(ONMI) $(OIPC) $(OMSG) $(OSEM) $(OSHM) $(OKMA) $(OFILE)

$(OFILE):	$(FILES)
	$(LD) -r $(FILES) -o $(OFILE)


$(OFP):         fp.o
	$(LD) -r -o $@ fp.o

$(ONMI):	nmi.o
	$(LD) -r -o $@ nmi.o

$(OIPC):	ipc.o
	$(LD) -r -o $@ ipc.o

$(OMSG):	msg.o
	$(LD) -r -o $@ msg.o

$(OSEM):	sem.o
	$(LD) -r -o $@ sem.o

$(OSHM):	shm.o
	$(LD) -r -o $@ shm.o

$(OKMA):	kma.o
	$(LD) -r -o $@ kma.o

clean:
	-rm -f $(FILES) fp.o nmi.o ipc.o msg.o shm.o sem.o kma.o

clobber:	clean
	-rm -f $(OFILE) $(OFP) $(ONMI) $(OIPC) $(OMSG) $(OSEM) $(OSHM) $(OKMA)

FRC:

#
# Header dependencies
#

acct.o: acct.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/resource.h \
	$(INCRT)/sys/uio.h \
	$(FRC)

bio.o: bio.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/vm/page.h \
	$(FRC)

bitmap.o: bitmap.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/bitmap.h \
	$(FRC)

bitmasks.o: bitmasks.c \
	$(FRC)

bs.o: bs.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/bootinfo.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

clock.o: clock.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/callo.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/evecb.h \
	$(INCRT)/sys/hrtcntl.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/class.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/vm/anon.h \
	$(FRC)

cmn_err.o: cmn_err.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/exec.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/bootinfo.h \
	$(INCRT)/sys/xdebug.h \
	$(INCRT)/sys/iobuf.h \
	$(FRC)

core.o: core.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/siginfo.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/fault.h \
	$(INCRT)/sys/syscall.h \
	$(INCRT)/sys/ucontext.h \
	$(INCRT)/sys/sigaction.h \
	$(INCRT)/sys/procfs.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/exec.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/vm/as.h \
	$(FRC)

cred.o: cred.c \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/fault.h \
	$(INCRT)/sys/syscall.h \
	$(INCRT)/sys/procfs.h \
	$(INCRT)/sys/dl.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/tuneable.h \
	$(FRC)

cxenix.o: cxenix.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/reg.h \
	$(FRC)

exec.o: exec.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/fp.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/exec.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/vm.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/x.out.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/weitek.h \
	$(INCRT)/vm/vm_hat.h \
	$(INCRT)/vm/anon.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/seg_vn.h \
	$(FRC)

execargs.o: execargs.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/vmsystm.h \
	$(INCRT)/vm/faultcatch.h \
	$(FRC)

ddi.o: ddi.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/ddi.h \
	$(FRC)

exit.o: exit.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/fp.h \
	$(INCRT)/sys/procfs.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/wait.h \
	$(INCRT)/sys/procset.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/class.h \
	$(INCRT)/sys/weitek.h \
	$(INCRT)/sys/events.h \
	$(INCRT)/sys/evsys.h \
	$(INCRT)/sys/hrtsys.h \
	$(INCRT)/vm/seg.h \
	$(FRC)

fbio.o: fbio.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fbuf.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_kmem.h \
	$(INCRT)/vm/seg_map.h \
	$(FRC)

fio.o: fio.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/evecb.h \
	$(INCRT)/sys/hrtcntl.h \
	$(INCRT)/sys/priocntl.h \
	$(INCRT)/sys/procset.h \
	$(INCRT)/sys/events.h \
	$(INCRT)/sys/evsys.h \
	$(INCRT)/sys/asyncsys.h \
	$(FRC)

flock.o: flock.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/tuneable.h \
	$(FRC)

fork.o: fork.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/fp.h \
	$(INCRT)/sys/weitek.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/class.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/v86.h \
	$(INCRT)/vm/vm_hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg_u.h \
	$(FRC)

grow.o: grow.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/bitmasks.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/vm.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/vm/vm_hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_dev.h \
	$(INCRT)/vm/seg_vn.h \
	$(FRC)

ipc.o: ipc.c \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/ipc.h \
	$(FRC)

kma.o: kma.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/systm.h \
	$(FRC)

kperf.o: kperf.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/disp.h \
	$(FRC)

list.o: list.c \
	$(INCRT)/sys/list.h \
	$(FRC)

local.o: local.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(FRC)

lock.o: lock.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/lock.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/vm/vm_hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(FRC)

xmmu.o: xmmu.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(FRC)

xsys.o: xsys.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/locking.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/timeb.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/proctl.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

machdep.o: machdep.c \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/v86.h \
	$(INCRT)/sys/fp.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/weitek.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

main.o: main.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/evecb.h \
	$(INCRT)/sys/hrtcntl.h \
	$(INCRT)/sys/hrtsys.h \
	$(INCRT)/sys/priocntl.h \
	$(INCRT)/sys/procset.h \
	$(INCRT)/sys/events.h \
	$(INCRT)/sys/evsys.h \
	$(INCRT)/sys/asyncsys.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/x.out.h \
	$(INCRT)/sys/weitek.h \
	$(INCRT)/sys/fp.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/sys/dmaable.h \
	$(FRC)

malloc.o: malloc.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

mapfile.o: mapfile.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/sysi86.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/sys/mman.h \
	$(FRC)

mod.o:	mod.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

move.o: move.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/vm/faultcatch.h \
	$(FRC)

msg.o: msg.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/msg.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/disp.h \
	$(FRC)

name.o: name.c 
	$(CC) $(CFLAGS) -c name.c \
		-DREL=`expr '"$(REL)' : '\(..\{0,8\}\)'`\" \
		-DVER=`expr '"$(VER)' : '\(..\{0,8\}\)'`\"

oem.o: oem.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/x.out.h \
	$(INCRT)/sys/weitek.h \
	$(INCRT)/sys/fp.h \
	$(INCRT)/sys/cram.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

pgrp.o: pgrp.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/procfs.h \
	$(FRC)

pipe.o: pipe.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fs/fifonode.h \
	$(INCRT)/sys/stream.h \
	$(FRC)

pmstub.o: pmstub.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

procset.o: procset.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/procset.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/wait.h \
	$(INCRT)/sys/procfs.h \
	$(FRC)

scalls.o: scalls.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/clock.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/uadmin.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/ustat.h \
	$(INCRT)/sys/statvfs.h \
	$(INCRT)/sys/procfs.h \
	$(INCRT)/sys/procset.h \
	$(INCRT)/sys/evecb.h \
	$(INCRT)/sys/hrtcntl.h \
	$(INCRT)/vm/vm_hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/faultcatch.h \
	$(FRC)

sched.o: sched.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/fp.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/class.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

session.o: session.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/evecb.h \
	$(INCRT)/sys/hrtcntl.h \
	$(INCRT)/sys/priocntl.h \
	$(INCRT)/sys/events.h \
	$(INCRT)/sys/evsys.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/session.h \
	$(INCRT)/sys/priocntl.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/vm/faultcatch.h \
	$(FRC)

sem.o: sem.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/sem.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/sysinfo.h \
	$(FRC)

shm.o: shm.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/shm.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vm.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/vm/vm_hat.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/vm/anon.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/vpage.h \
	$(FRC)

sig.o: sig.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/procfs.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/fp.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/debugreg.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/weitek.h \
	$(INCRT)/sys/v86.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/wait.h \
	$(INCRT)/sys/class.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/procset.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/evecb.h \
	$(INCRT)/sys/hrtcntl.h \
	$(INCRT)/sys/events.h \
	$(INCRT)/sys/evsys.h \
	$(INCRT)/vm/as.h \
	$(FRC)

slp.o: slp.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/class.h \
	$(INCRT)/sys/priocntl.h \
	$(INCRT)/sys/evecb.h \
	$(INCRT)/sys/hrtcntl.h \
	$(INCRT)/sys/events.h \
	$(INCRT)/sys/evsys.h \
	$(FRC)

startup.o: startup.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/class.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/bootinfo.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_kmem.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/vm/seg_u.h \
	$(INCRT)/vm/seg_map.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/vm/faultcatch.h \
	$(FRC)

streamio.o: streamio.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/strstat.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/poll.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/sad.h \
	$(INCRT)/sys/priocntl.h \
	$(INCRT)/sys/hrtcntl.h \
	$(INCRT)/sys/procset.h \
	$(INCRT)/sys/events.h \
	$(INCRT)/sys/evsys.h \
	$(FRC)

strsubr.o: strsubr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/strstat.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/poll.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/sad.h \
	$(INCRT)/sys/priocntl.h \
	$(INCRT)/sys/hrtcntl.h \
	$(INCRT)/sys/procset.h \
	$(INCRT)/sys/events.h \
	$(INCRT)/sys/evsys.h \
	$(INCRT)/sys/session.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/disp.h \
	$(FRC)

subr.o: subr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/procfs.h \
	$(INCRT)/sys/dl.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/tuneable.h \
	$(FRC)

sysent.o: sysent.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(FRC)

sysi86.o: sysi86.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysi86.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/uadmin.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fp.h \
	$(INCRT)/sys/rtc.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/xdebug.h \
	$(INCRT)/sys/weitek.h \
	$(FRC)

todc.o: todc.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/rtc.h \
	$(FRC)

trap.o: trap.c \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/trap.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fault.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/procfs.h \
	$(INCRT)/sys/prsystm.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debugreg.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/class.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/pic.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/v86.h \
	$(INCRT)/sys/weitek.h \
	$(INCRT)/sys/xdebug.h \
	$(INCRT)/sys/fp.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/vmsystm.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/evecb.h \
	$(INCRT)/sys/hrtcntl.h \
	$(INCRT)/sys/priocntl.h \
	$(INCRT)/sys/events.h \
	$(INCRT)/sys/evsys.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/vm/seg_kmem.h \
	$(INCRT)/vm/faultcode.h \
	$(INCRT)/vm/faultcatch.h \
	$(FRC)

vm_meter.o: vm_meter.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/vm/kernel.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/vm.h \
	$(INCRT)/vm/vm_hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/rm.h \
	$(INCRT)/sys/var.h \
	$(FRC)

vm_pageout.o: vm_pageout.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/vm.h \
	$(INCRT)/sys/vmparam.h \
	$(INCRT)/vm/trace.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/vm/vm_hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

vm_subr.o: vm_subr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/vm.h \
	$(INCRT)/vm/vm_hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/vm/seg_kmem.h \
	$(INCRT)/vm/seg_u.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

fp.o:	fp.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/trap.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/fp.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	../fp/sizes.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/exec.h \
	$(INCRT)/sys/weitek.h \
	$(INCRT)/vm/faultcatch.h \
	$(FRC)

nmi.o:	nmi.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/bootinfo.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/vm/faultcatch.h \
	$(INCRT)/sys/user.h \
	$(FRC)

predki.o:	predki.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/ddi.h \
	$(FRC)


pid.o:	pid.c \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/session.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(FRC)
