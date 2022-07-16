#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# 		PROPRIETARY NOTICE (Combined)
# 
# This source code is unpublished proprietary information
# constituting, or derived under license from AT&T's UNIX(r) System V.
# In addition, portions of such source code were derived from Berkeley
# 4.3 BSD under license from the Regents of the University of
# California.
# 
# 
# 
# 		Copyright Notice 
# 
# Notice of copyright on this source code product does not indicate 
# publication.
# 
# 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
# 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
# 	          All rights reserved.
#  

#ident	"@(#)kern-vm:vm.mk	1.3.1.4"

INCRT = ..
OFILE = $(CONF)/pack.d/kernel/vm.o
DASHO = -O
CFLAGS = -I$(INCRT) -D_KERNEL -Dnotnow $(MORECPP) $(DASHO) 
PFLAGS = -I$(INCRT) -D_KERNEL $(MORECPP)
FRC =

FILES = \
	seg_dev.o \
	seg_dummy.o \
	seg_objs.o \
	seg_kmem.o \
	seg_map.o \
	seg_u.o \
	seg_vn.o \
	seg_vpix.o \
	vm_anon.o \
	vm_as.o \
	vm_hat.o \
	vm_machdep.o \
	vm_mp.o \
	vm_page.o \
	vm_pvn.o \
	vm_rm.o \
	vm_seg.o \
	vm_swap.o \
	vm_vpage.o \
	memprobe.o \
	ucopy.o


all:		$(OFILE)


$(OFILE):	$(FILES)
		$(LD) -r $(FILES) -o $(OFILE)

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(OFILE)

FRC:

seg_objs.o: seg_objs.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_objs.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/vm/vpage.h \
	$(FRC)

seg_dev.o: seg_dev.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_dev.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/vm/vpage.h \
	$(FRC)

seg_dummy.o: seg_dummy.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(FRC)

seg_kmem.o: seg_kmem.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/vm.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/vm/seg_kmem.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/anon.h \
	$(INCRT)/vm/rm.h \
	$(INCRT)/vm/page.h \
	$(FRC)

seg_map.o: seg_map.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/vm/trace.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/vm/seg_kmem.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_map.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/vm/rm.h \
	$(FRC)

seg_u.o: seg_u.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/vm/anon.h \
	$(INCRT)/vm/rm.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/vm/vm_hat.h \
	$(INCRT)/vm/seg_u.h \
	$(INCRT)/sys/proc.h \
	$(FRC)

seg_vn.o: seg_vn.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/vmsystm.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/vm/trace.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/mp.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/vm/anon.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/vpage.h \
	$(INCRT)/vm/seg_kmem.h \
	$(FRC)

seg_vpix.o: seg_vpix.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/vmsystm.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/vm/trace.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/mp.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_vpix.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/vm/anon.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/vpage.h \
	$(INCRT)/vm/seg_kmem.h \
	$(FRC)

vm_anon.o: vm_anon.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/vmmeter.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/anon.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/vm/rm.h \
	$(INCRT)/vm/mp.h \
	$(INCRT)/vm/trace.h \
	$(FRC)

vm_as.o: vm_as.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/vm/seg_kmem.h \
	$(FRC)

vm_machdep.o: vm_machdep.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/vm.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/vm/seg_kmem.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/vm/cpu.h \
	$(FRC)

vm_hat.o: vm_hat.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/vm/vm_hat.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/bitmasks.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/disp.h \
	$(FRC)

vm_page.o: vm_page.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/vm.h \
	$(INCRT)/vm/trace.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/anon.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/vm/mp.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/user.h \
	$(FRC)

vm_pvn.o: vm_pvn.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/vmmeter.h \
	$(INCRT)/sys/vmsystm.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/vm/trace.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/rm.h \
	$(INCRT)/vm/pvn.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/seg_map.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/disp.h \
	$(FRC)

vm_rm.o: vm_rm.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/rm.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/page.h \
	$(FRC)

vm_seg.o: vm_seg.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/mp.h \
	$(FRC)

vm_swap.o: vm_swap.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/vm/bootconf.h \
	$(INCRT)/vm/trace.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/anon.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/vm/seg_map.h \
	$(FRC)

vm_vpage.o: vm_vpage.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/vm/vpage.h \
	$(INCRT)/vm/mp.h \
	$(FRC)

memprobe.o: memprobe.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/vm/faultcatch.h \
	$(FRC)

ucopy.o: ucopy.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/vmsystm.h \
	$(INCRT)/vm/faultcatch.h \
	$(FRC)
