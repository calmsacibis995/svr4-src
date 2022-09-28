#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ufs.cmds:ufs/ufsdump/ufsdump.mk	1.12.3.1"

#       dump.h                  header file
#       dumpitime.c             reads /etc/dumpdates
#       dumpmain.c              driver
#       dumpoptr.c              operator interface
#       dumptape.c              handles the mag tape and opening/closing
#       dumptraverse.c          traverses the file system
#       unctime.c               undo ctime
#
# CPPFLAGS:
#       DEBUG                   use local directory to find ddate and dumpdates
#       TDEBUG                  trace out the process forking
#
BINS= ufsdump
OBJS= dumpitime.o dumpmain.o dumpoptr.o dumptape.o \
	dumptraverse.o unctime.o
SRCS= dumpitime.c dumpmain.c dumpoptr.c dumptape.c \
	dumptraverse.c unctime.c
HDRS= dump.h
ROOT=
INC= $(ROOT)/usr/include
TESTDIR = .
INSDIR1 = $(ROOT)/usr/lib/fs/ufs
INSDIR2 = $(ROOT)/usr/sbin
CFLAGS= -O -I$(INC)
CC= cc
LD= ld
INS= install
LDFLAGS= -s

all: $(BINS) install clean

$(BINS): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o ufsdump $(OBJS) $(SHLIBS)

dumpitime.o:	dumpitime.c
	$(CC) $(CFLAGS) -c dumpitime.c

dumpmain.o:	dumpmain.c
	$(CC) $(CFLAGS) -c dumpmain.c

dumpoptr.o:	dumpoptr.c
	$(CC) $(CFLAGS) -c dumpoptr.c

dumptape.o:	dumptape.c
	$(CC) $(CFLAGS) -c dumptape.c

dumptraverse.o:	dumptraverse.c
	$(CC) $(CFLAGS) -c dumptraverse.c

unctime.o:	unctime.c
	$(CC) $(CFLAGS) -c unctime.c

install: $(BINS)
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(INSDIR1) ]; \
		then \
		mkdir $(INSDIR1); \
		fi;
	$(INS) -f $(INSDIR1) -m 0555 -u bin -g bin $(TESTDIR)/ufsdump
	-rm -f $(INSDIR2)/ufsdump
	ln $(INSDIR1)/ufsdump $(INSDIR2)/ufsdump
	
clean:
	rm -f $(BINS) $(OBJS)

clobber: clean
	rm -f $(BINS)
	
