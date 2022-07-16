#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucblibc:i386/makefile	1.1.2.1"
#
#  makefile for ucblibc/i386
#
#
MAC=
M4=m4 m4.def
CC=cc
CFLAGS=-O
PROF=
NONPROF=
DEFLIST=
SDEFLIST=

OBJECTS=

POBJECTS=
TSORT=$(PFX)tsort

ALL:
	cd gen; make -e
	cd sys; make -e

archive:
	#
	# Note that "archive" is invoked with libc/object as current directory.
	#
	$(LORDER) *.o|$(TSORT) >objlist
	$(AR) q ../lib.libucb `cat objlist`
clean:
	-rm -f *.o

clobber: clean
	-rm -f */*.o
