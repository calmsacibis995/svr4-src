#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)bdiff:bdiff.mk	1.12.1.1"
#
#

ROOT= 

USRBIN=$(ROOT)/usr/bin
INC=$(ROOT)/usr/include
INS=install

CFLAGS=-c -I$(INC)

LDFLAGS= -s

all:	bdiff

bdiff:	bdiff.o
	$(CC) $(LDFLAGS) -o bdiff bdiff.o $(SHLIBS)

bdiff.o:	bdiff.c			\
			$(INC)/fatal.h	\
			$(INC)/signal.h	\
			$(INC)/sys/types.h	\
			$(INC)/unistd.h	\
			$(INC)/stdio.h	\
			$(INC)/ctype.h
	$(CC) $(CFLAGS) bdiff.c

install:	all
	$(INS) -f $(USRBIN) -m 0555 -u bin -g bin bdiff

clean:
	-rm -f bdiff.o

clobber:	clean
	-rm -f bdiff
