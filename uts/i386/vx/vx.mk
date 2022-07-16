#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-vx:vx.mk	1.3"

INCRT = ..
CFLAGS = -I$(INCRT) -D_KERNEL -O $(MORECPP)
OFILE = $(CONF)/pack.d/vx/Driver.o
FRC =

FILES = v86subr.o v86.o

all:    $(OFILE)

$(OFILE): v86subr.o v86.o
	$(LD) -r $(FILES) -o $(OFILE)

clean:
	-rm -f $(FILES)

clobber:	clean
	-rm -f $(OFILE)

FRC:

#
# Header dependencies
#

v86subr.o: v86subr.c \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/v86.h \
	$(INCRT)/sys/fp.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/weitek.h

v86.o: v86.c \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/fp.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/v86.h
