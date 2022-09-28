#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ufs.cmds:ufs/quotaon/quotaon.mk	1.7.3.1"

ROOT=
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/lib/fs/ufs
DIRDIR = $(ROOT)/usr/lib/fs
INSDIR2 = $(ROOT)/usr/sbin
CFLAGS = -O
LDFLAGS = -s
INS=install
OBJS=

all:  install clobber

quotaon: quotaon.c $(OBJS)
	$(CC) -I$(INC) $(CFLAGS) $(LDFLAGS) -o quotaon quotaon.c $(OBJS) $(SHLIBS)

install: quotaon
	if [ ! -d $(DIRDIR) ]; \
	then \
		mkdir $(DIRDIR); \
	fi
	if [ ! -d $(INSDIR) ]; \
	then \
		mkdir $(INSDIR); \
	fi
	rm -f $(INSDIR)/quotaon
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin quotaon
	-rm -f $(INSDIR)/quotaoff
	ln $(INSDIR)/quotaon $(INSDIR)/quotaoff
	-rm -f $(INSDIR2)/quotaon
	ln $(INSDIR)/quotaon $(INSDIR2)/quotaon
	-rm -f $(INSDIR2)/quotaoff
	ln $(INSDIR)/quotaon $(INSDIR2)/quotaoff
	
clean:
	-rm -f quotaon.o

clobber: clean
	rm -f quotaon
