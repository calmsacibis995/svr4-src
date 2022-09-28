#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ufs.cmds:ufs/repquota/repquota.mk	1.7.3.1"

ROOT=
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/lib/fs/ufs
DIRDIR = $(ROOT)/usr/lib/fs
INSDIR2 = $(ROOT)/usr/sbin
SYMLINK = :
CFLAGS = -O
LDFLAGS = -s
INS=install
OBJS=

all:  install clobber

repquota: repquota.c $(OBJS)
	$(CC) -I$(INC) $(CFLAGS) $(LDFLAGS) -o repquota repquota.c $(OBJS) $(SHLIBS)

install: repquota
	if [ ! -d $(DIRDIR) ]; \
	then \
		mkdir $(DIRDIR); \
	fi
	if [ ! -d $(INSDIR) ]; \
	then \
		mkdir $(INSDIR); \
	fi
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin repquota
	-rm -f $(INSDIR2)/repquota
	ln $(INSDIR)/repquota $(INSDIR2)/repquota
	$(SYMLINK) $(INSDIR)/repquota /usr/bin/repquota
	
clean:
	-rm -f repquota.o

clobber: clean
	rm -f repquota
