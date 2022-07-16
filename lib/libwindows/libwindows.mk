#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)attwin:lib/libagent/makefile	1.8.2.1"
#
#		Copyright 1985 AT&T
#

CC = cc
AR = ar
USRLIB = $(ROOT)/usr/lib
CFLAGS = -O -I$(INC)
INC = $(ROOT)/usr/include
INS = install

all:	libwindows.a

libwindows.a:	libwindows.o
	$(AR) r libwindows.a libwindows.o

libwindows.o:	libwindows.c
libwindows.o:	$(INC)/stdio.h
libwindows.o:	$(INC)/fcntl.h
libwindows.o:	$(INC)/windows.h
libwindows.o:	$(INC)/sys/types.h
libwindows.o:	$(INC)/sys/tty.h
libwindows.o:	$(INC)/sys/jioctl.h
libwindows.o:	$(INC)/sys/nxtproto.h
	$(CC) $(CFLAGS) -c libwindows.c

install:	all
	$(INS) -f $(USRLIB) -u bin -g bin -m 644 libwindows.a

clean:
	rm -f libwindows.o

clobber: clean
	rm -f libwindows.a
