#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ufs.cmds:ufs/quotacheck/quotacheck.mk	1.5.3.1"

ROOT=
INCSYS = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/lib/fs/ufs
DIRDIR = $(ROOT)/usr/lib/fs
INSDIR2 = $(ROOT)/usr/sbin
CFLAGS = -O
LDFLAGS = -s
INS=install
OBJS=

all:  install clobber

quotacheck: quotacheck.c $(OBJS)
	$(CC) -I$(INCSYS) $(CFLAGS) $(LDFLAGS) -o quotacheck quotacheck.c $(OBJS) $(SHLIBS)

install: quotacheck
	if [ ! -d $(DIRDIR) ]; \
	then \
		mkdir $(DIRDIR); \
	fi
	if [ ! -d $(INSDIR) ]; \
	then \
		mkdir $(INSDIR); \
	fi
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin quotacheck
	-rm -f $(INSDIR2)/quotacheck
	ln $(INSDIR)/quotacheck $(INSDIR2)/quotacheck
	
clean:
	-rm -f quotacheck.o

clobber: clean
	rm -f quotacheck
