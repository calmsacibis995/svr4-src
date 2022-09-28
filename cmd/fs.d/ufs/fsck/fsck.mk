#ident	"@(#)fsck.mk	1.2	91/09/21	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ufs.cmds:ufs/fsck/fsck.mk	1.5.4.1"

TESTDIR = .
INSDIR1 = $(ROOT)/usr/lib/fs/ufs
INSDIR2 = $(ROOT)/etc/fs/ufs
INS = install
CFLAGS = -O -I$(INC)
LDFLAGS =
INC = $(ROOT)/usr/include
OBJS=   dir.o inode.o pass1.o pass1b.o pass2.o \
	pass3.o pass4.o pass5.o setup.o utilities.o
UFSOBJS= ufs_subr.o ufs_tables.o
UFSDIR= $(ROOT)/usr/src/uts/i386/fs/ufs

#all:  install clobber
all: install

fsck: $(OBJS) $(UFSOBJS) main.o
	$(CC) -I$(INC) $(CFLAGS) $(LDFLAGS) -o fsck main.c $(OBJS) $(UFSOBJS) $(ROOTLIBS)
dir.o:	dir.c
	$(CC) -I$(INC) $(CFLAGS) $(LDFLAGS) -c dir.c

ufs_subr.o:     $(UFSDIR)/ufs_subr.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $(UFSDIR)/ufs_subr.c

ufs_tables.o:   $(UFSDIR)/ufs_tables.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $(UFSDIR)/ufs_tables.c
	
install: fsck
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(INSDIR1) ]; \
		then \
		mkdir $(INSDIR1); \
		fi;
	@if [ ! -d $(ROOT)/etc/fs ]; \
		then \
		mkdir $(ROOT)/etc/fs; \
		fi;
	@if [ ! -d $(INSDIR2) ]; \
		then \
		mkdir $(INSDIR2); \
		fi;
	$(INS) -f $(INSDIR1) -m 0555 -u bin -g bin $(TESTDIR)/fsck
	$(INS) -f $(INSDIR2) -m 0555 -u bin -g bin $(TESTDIR)/fsck


clean:     
	-rm -f $(OBJS) $(UFSOBJS) main.o
	
clobber: clean
	rm -f fsck

