#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ucblibc:i386/sys/makefile	1.1.3.1"
#
#
# makefile for ucblib/libc/i386/sys
#

CC=cc
CFLAGS=-c

OBJECTS= setjmp.o syscall.o _times.o

SOURCES= setjmp.s syscall.s _times.s

.s.o:
	$(CC) $(CFLAGS) $*.s

ALL:	$(OBJECTS)

clean:
	rm -rf *.o

clobber:
	rm -rf *.o
