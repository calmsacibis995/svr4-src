#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)proc.cmds:proc.mk	1.13.1.1"

ROOT =
TESTDIR = .
INSDIR = $(ROOT)/etc/fs/proc
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
	@if [ ! -d $(ROOT)/usr/lib/fs/proc ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs/proc; \
		fi;
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin $(TESTDIR)/mount
	$(INS) -f  $(ROOT)/usr/lib/fs/proc -m 0555 -u bin -g bin $(TESTDIR)/mount

clean:
	rm -f *.o

clobber:	clean
	rm -f $(TESTDIR)/mount
FRC:
