#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ufs.cmds:ufs/tunefs/tunefs.mk	1.5.3.1"

ROOT=
INC = $(ROOT)/usr/include
TESTDIR = .
INSDIR = $(ROOT)/usr/lib/fs/ufs
DIRDIR = $(ROOT)/usr/lib/fs
INSDIR2 = $(ROOT)/usr/sbin
CFLAGS = -O
LDFLAGS = -s
INS=install
OBJS=

all:  install clobber

tunefs: tunefs.c $(OBJS)
	$(CC) -I$(INC) $(CFLAGS) $(LDFLAGS) -o tunefs tunefs.c $(OBJS) $(SHLIBS)

install: tunefs
	if [ ! -d $(DIRDIR) ]; \
	then \
		mkdir $(DIRDIR); \
	fi
	if [ ! -d $(INSDIR) ]; \
	then \
		mkdir $(INSDIR); \
	fi
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin $(TESTDIR)/tunefs
	-rm -f $(INSDIR2)/tunefs
	ln $(INSDIR)/tunefs $(INSDIR2)/tunefs
	
clean:
	-rm -f tunefs.o

clobber: clean
	rm -f tunefs
