#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-fs:fs.mk	1.3.1.1"

STRIP = strip
INCRT = ..
CC = cc
OFILE = $(CONF)/pack.d/kernel/fs.o

DASHG =
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS)
DEFLIST =
FRC =

FILES = \
	dnlc.o \
	fs_subr.o \
	fsflush.o \
	lookup.o \
	pathname.o \
	strcalls.o \
	vfs.o \
	vncalls.o \
	vnode.o

all:	$(OFILE) fstypes

$(OFILE): $(FILES)
	$(LD) -r -o $(OFILE) $(FILES)

.c.o:
	$(CC) $(DEFLIST) -I$(INCRT) $(CFLAGS) -c $*.c

fstypes:
	@for i in *;\
	do\
		if [ "${NONETWORK}" = "nonet" ] && [ $$i = nfs -o $$i = rfs ]; \
		then \
			continue; \
		elif [ -d $$i -a -f $$i/$$i.mk ];then\
			case $$i in\
			*.*)\
				;;\
			*)\
			cd  $$i;\
			echo "====== $(MAKE) -f $$i.mk \"MAKE=$(MAKE)\" \"AS=$(AS)\" \"CC=$(CC)\" \"LD=$(LD)\" \"FRC=$(FRC)\" \"MORECPP=$(MORECPP)\"";\
		$(MAKE) -f $$i.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)"; \
			cd .. ;; \
			esac;\
		fi;\
	done

clean:
	-rm -f *.o
	@for i in *; \
	do \
		if [ -d $$i -a -f $$i/$$i.mk ]; then \
			cd $$i; \
			$(MAKE) -f $$i.mk clean; \
			cd ..; \
		fi; \
	done

clobber:	clean
	-rm -f $(OFILE)
	@for i in *; \
	do \
		if [ -d $$i -a -f $$i/$$i.mk ]; then \
			cd $$i; \
			$(MAKE) -f $$i.mk clobber; \
			cd ..; \
		fi; \
	done

#
# Header dependencies
#

dnlc.o: dnlc.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/dnlc.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

fs_subr.o: fs_subr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/statvfs.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/unistd.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/poll.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/list.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/rf_comm.h \
	$(INCRT)/fs/fs_subr.h \
	$(FRC)

fsflush.o: fsflush.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/vm.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/vm/pvn.h \
	$(FRC)

lookup.o: lookup.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/disp.h \
	$(FRC)

pathname.o: pathname.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/vnode.h \
	$(FRC)

strcalls.o: strcalls.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(FRC)

vfs.o: vfs.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/statvfs.h \
	$(INCRT)/sys/statfs.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/dnlc.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

vncalls.o: vncalls.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/mode.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/poll.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/filio.h \
	$(INCRT)/sys/locking.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/mkdev.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/unistd.h \
	$(FRC)

vnode.o: vnode.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
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
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/mode.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/systm.h \
	$(FRC)

