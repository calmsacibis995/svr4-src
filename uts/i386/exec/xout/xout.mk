#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-exe:xout/xout.mk	1.3"

STRIP = strip
INCRT = ../..
OFILE = $(CONF)/pack.d/xout/Driver.o

DASHG =
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) -I$(INCRT) $(PFLAGS)
FRC =


all:	$(OFILE)

$(OFILE):	xout.o
	cp xout.o $(OFILE)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(OFILE)

#
# Header dependencies
#


xout.o: xout.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/pathname.h \
	$(INCRT)/sys/x.out.h \
	$(INCRT)/sys/exec.h \
	$(INCRT)/vm/seg_vn.h \
	$(INCRT)/vm/as.h \
	$(FRC)

