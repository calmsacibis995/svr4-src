#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rmntstat:rmntstat.mk	1.3.10.1"


ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/sbin
INC = $(ROOT)/usr/include
INS = install
SYMLINK = :
CFLAGS = -O -s -I$(INC)
FRC =

all: rmntstat

rmntstat: rmntstat.o
	$(CC) $(CFLAGS) -o $(TESTDIR)/rmntstat rmntstat.o $(SHLIBS)

rmntstat.o : rmntstat.c  \
	$(INC)/stdio.h \
	$(INC)/sys/fcntl.h \
	$(INC)/sys/types.h \
	$(INC)/sys/nserve.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/rf_sys.h
	$(CC) -I$(INC) -c $(CFLAGS) rmntstat.c

install: all
	-rm -f $(ROOT)/usr/bin/rmntstat
	$(INS) -f $(INSDIR) -m 555 -u bin -g bin rmntstat
	-$(SYMLINK) /usr/sbin/rmntstat $(ROOT)/usr/bin/rmntstat

clean:
	rm -f rmntstat.o

clobber: clean
	rm -f $(TESTDIR)/rmntstat
FRC:

