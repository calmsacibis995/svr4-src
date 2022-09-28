#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ufs.cmds:ufs/labelit/labelit.mk	1.3.3.1"

ROOT=
INC = $(ROOT)/usr/include
TESTDIR = .
INSDIR1 = $(ROOT)/usr/lib/fs/ufs
INSDIR2 = $(ROOT)/etc/fs/ufs
INS = install
CFLAGS = -O -I$(INC)
LDFLAGS = -s


all:  install clobber

labelit: labelit.c $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o labelit labelit.c $(OBJS) $(SHLIBS)

install: labelit
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(INSDIR1) ]; \
		then \
		mkdir $(INSDIR1); \
		fi;
	$(INS) -f $(INSDIR1) -m 0555 -u bin -g bin $(TESTDIR)/labelit

clean:
	-rm -f labelit.o

clobber: clean
	rm -f labelit
