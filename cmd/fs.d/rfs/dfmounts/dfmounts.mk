#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfs.cmds:dfmounts/dfmounts.mk	1.5.3.1"


ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/lib/fs/rfs
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
INS = install
CFLAGS = -O -s
FRC =

all: dfmounts

dfmounts: dfmounts.o
	$(CC) $(CFLAGS) -o $(TESTDIR)/dfmounts dfmounts.o $(SHLIBS)

dfmounts.o : dfmounts.c  \
	$(INC)/stdio.h \
	$(INCSYS)/sys/fcntl.h \
	$(INCSYS)/sys/types.h \
	$(INCSYS)/sys/nserve.h \
	$(INCSYS)/sys/stat.h \
	$(INCSYS)/sys/rf_sys.h
	$(CC) -I$(INC) -I$(INCSYS) -c $(CFLAGS) dfmounts.c

install: all
	@if [ ! -d $(INSDIR) ]; then mkdir -p $(INSDIR); fi;
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin $(TESTDIR)/dfmounts

clean:
	rm -f dfmounts.o

clobber: clean
	rm -f $(TESTDIR)/dfmounts
FRC:

