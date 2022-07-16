#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kdb:gdebugger/gdebugger.mk	1.1"

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
OFILE = $(CONF)/pack.d/gdebugger/Driver.o

FILES = dbg.o

all:		$(OFILE)

$(OFILE):	$(FILES) $(FRC)
	$(LD) -r -o $@ $(FILES)

.c.o:
	$(CC) $(DEFLIST) -I$(INCRT) $(CFLAGS) -c $*.c

clean:
	rm -f *.o

clobber:	clean
	rm -f $(OFILE)

FRC:


#
# Header Dependencies
#

dbg.o:	dbg.c \
	$(INCRT)/sys/kdebugger.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/regset.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/xdebug.h \
	$(FRC)
