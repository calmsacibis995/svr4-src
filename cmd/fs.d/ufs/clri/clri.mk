#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ufs.cmds:ufs/clri/clri.mk	1.5.3.1"

TESTDIR = .
ROOT =
INSDIR1 = $(ROOT)/usr/lib/fs/ufs
INS = install
CFLAGS = -O
LDFLAGS = -s
INC = $(ROOT)/usr/include

all:  install clobber

clri: clri.c $(OBJS)
	$(CC) -I$(INC) $(CFLAGS) $(LDFLAGS) -o clri clri.c $(OBJS) $(SHLIBS)

install: clri
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(INSDIR1) ]; \
		then \
		mkdir $(INSDIR1); \
		fi;
	$(INS) -f $(INSDIR1) -m 0555 -u bin -g bin $(TESTDIR)/clri

clean:
	-rm -f clri.o

clobber: clean
	rm -f clri
