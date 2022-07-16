#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-exe:intp/intp.mk	1.3"

STRIP = strip
INCRT = ../..
OFILE = $(CONF)/pack.d/intp/Driver.o

DASHG =
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) -I$(INCRT) $(PFLAGS)
FRC =


all:	$(OFILE)

$(OFILE):	intp.o 
	cp intp.o $(OFILE)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(OFILE)

#
# Header dependencies
#


intp.o: intp.c \
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
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/exec.h \
	$(FRC)

