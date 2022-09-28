#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)pcintf:util/makefile	1.1"
#! /bin/make -f
#
# SCCSID(@(#)makefile	3.10	LCC);   /* Modified: 2/26/90 16:53:43 */
#
# NOTE: The 'unix2dos.c' and 'dos2unix.c' Unix versions here are
# supposed to be the same as the DOS versions (in /pci/dos/*/util).
# (Of course the unix versions have been run thru 'dos2unix'.)

IPATH = -I../bridge -I$(ROOT)/usr/include
CC = cc
CFLAGS = $(IPATH)
CC286 = cc
CFLAGS286 = -Me $(IPATH)
CC370 = cc
CFLAGS370 = $(IPATH)
CFLAGS386= $(IPATH)
CC386=cc

#---	TARGETS
generic: unix2dos dos2unix
286util: unix2dos.286 dos2unix.286
386util: unix2dos.386 dos2unix.386
370util: unix2dos.370 dos2unix.370

#---	CLEAN TARGETS
clean:
	-rm -rf *.o
386.clean:
	-rm -rf *.o
286.clean:
	-rm -rf *.o
370.clean:
	-rm -rf *.o

#---	CLOBBER TARGETS
clobber:
	-rm -rf *.o  unix2dos dos2unix
386.clobber:
	-rm -rf *.o *.386
286.clobber:
	-rm -rf *.o *.286
370.clobber:
	-rm -rf *.o *.370

#---	GENERIC RULES
unix2dos: unix2dos.c
	$(CC) $(CFLAGS) unix2dos.c -o unix2dos
dos2unix: dos2unix.c
	$(CC) $(CFLAGS) dos2unix.c -o dos2unix

#---	286 RULES
unix2dos.286: unix2dos.c
	$(CC286) $(CFLAGS286) unix2dos.c -o unix2dos.286

dos2unix.286: dos2unix.c
	$(CC286) $(CFLAGS286) dos2unix.c -o dos2unix.286

#---	386 RULES
unix2dos.386: unix2dos.c
	$(CC386) $(CFLAGS386) unix2dos.c -o unix2dos.386

dos2unix.386: dos2unix.c
	$(CC386) $(CFLAGS386) dos2unix.c -o dos2unix.386

#---	370 RULES
unix2dos.370: unix2dos.c
	$(CC370) $(CFLAGS370) unix2dos.c -o unix2dos.370

dos2unix.370: dos2unix.c
	$(CC370) $(CFLAGS370) dos2unix.c -o dos2unix.370

