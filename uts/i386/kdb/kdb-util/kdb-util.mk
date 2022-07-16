#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kdb:kdb-util/kdb-util.mk	1.2"

BUS = AT386
ARCH = AT386
MORECPP = -D$(BUS) -D$(ARCH)
DEFLIST =
FRC =

STRIP = strip
INCRT = ../..
CC = cc
DASHG =
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS)
CONF = $(ROOT)/etc/conf
OFILE = $(CONF)/pack.d/kdb-util/Driver.o

FILES = kdb.o db_as.o

all:	$(OFILE)

$(OFILE):	$(FILES)
	$(LD) -r -o $(OFILE) $(FILES)

.c.o:
	$(CC) $(DEFLIST) -I$(INCRT) $(CFLAGS) -c $*.c

clean:
	rm -f *.o

clobber: clean
	rm -f $(OFILE)

FRC:


#
# Header dependencies
#

kdb.o:	kdb.c \
	../db_as.h \
	$(INCRT)/sys/kdebugger.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/regset.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/xdebug.h \
	$(FRC)

db_as.o: db_as.c \
	../db_as.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/disp.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/as.h \
	$(FRC)
