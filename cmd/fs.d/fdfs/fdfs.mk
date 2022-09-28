#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)fdfs.cmds:fdfs.mk	1.1"

ROOT =
TESTDIR = .
#INSDIR = $(ROOT)/etc/fs/fd	Porting base
INSDIR = $(ROOT)/etc/fs/fdfs
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
INS = install
CFLAGS = -O -s
LDFLAGS =
FRC =

all:	mount 

mount:	mount.c\
	$(INC)/stdio.h\
	$(INC)/signal.h\
	$(INC)/unistd.h\
	$(INC)/errno.h\
	$(INCSYS)/sys/mnttab.h\
	$(INCSYS)/sys/mount.h\
	$(INCSYS)/sys/types.h\
	$(FRC)
	$(CC) -I$(INC) -I$(INCSYS) $(CFLAGS) -o $(TESTDIR)/mount mount.c $(LDFLAGS) $(ROOTLIBS)

install: all
	@if [ ! -d $(ROOT)/etc/fs ]; \
		then \
		mkdir $(ROOT)/etc/fs; \
		fi;
	@if [ ! -d $(INSDIR) ]; \
		then \
		mkdir $(INSDIR); \
		fi;
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(ROOT)/usr/lib/fs/fdfs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs/fdfs; \
		fi;
	$(INS) -f $(INSDIR) $(TESTDIR)/mount
	cp $(INSDIR)/mount $(ROOT)/usr/lib/fs/fdfs/mount
	cp /dev/null fsck
	$(INS) -f $(INSDIR) $(TESTDIR)/fsck
	cp $(INSDIR)/fsck  $(ROOT)/usr/lib/fs/fdfs/fsck

clean:
	rm -f *.o

clobber:	clean
	rm -f $(TESTDIR)/mount
FRC:
