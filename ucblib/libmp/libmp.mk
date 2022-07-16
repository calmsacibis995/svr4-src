#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucblibmp:libmp.mk	1.1.4.1"
#
# makefile for ucblibdbm.a
#
#

CC=cc
CFLAGS= -O
PROF=
NONPROF=
INC=$(ROOT)/usr/include
INC1=$(ROOT)/usr/ucbinclude
INCSYS=$(ROOT)/usr/include/sys
USRLIB=$(ROOT)/usr/ucblib
INS=install
LIBRARY = libmp.a
LORDER=$(PFX)lorder
TSORT=$(PFX)tsort


OBJECTS = gcd.o madd.o mdiv.o mout.o msqrt.o mult.o pow.o util.o

SOURCES =  gcd.c madd.c mdiv.c mout.c msqrt.c mult.c pow.c util.c

ALL:	$(LIBRARY)
	$(LORDER) *.o|$(TSORT) >objlist
	$(AR) cr libmp.a `cat objlist`

$(LIBRARY):	$(OBJECTS)

gcd.o:	$(INC1)/mp.h
	$(CC) $(CFLAGS) -I$(INC1) -c gcd.c

madd.o:	$(INC1)/mp.h
	$(CC) $(CFLAGS) -I$(INC1) -c madd.c

mdiv.o:	$(INC1)/mp.h \
	$(INC)/stdio.h
	$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c mdiv.c

mout.o:	$(INC1)/mp.h \
	$(INC)/stdio.h
	$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c mout.c

msqrt.o:	$(INC1)/mp.h
		$(CC) $(CFLAGS) -I$(INC1) -c msqrt.c

mult.o:	$(INC1)/mp.h
	$(CC) $(CFLAGS) -I$(INC1) -c mult.c


pow.o:	$(INC1)/mp.h
	$(CC) $(CFLAGS) -I$(INC1) -c pow.c

util.o:	$(INC1)/mp.h \
	$(INC)/stdio.h
	$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c util.c

GLOBALINCS = $(INC1)/mp.h $(INC)/stdio.h

install: ALL
	$(INS) -f $(USRLIB) -m 644 $(LIBRARY)

clean:
	rm -f $(OBJECTS) objlist

clobber: clean
	rm -f $(LIBRARY)
