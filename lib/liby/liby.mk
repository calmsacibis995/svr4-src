#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)liby:liby.mk	1.10"

ROOT=
SGS=
OWN=bin
GRP=bin
CC=cc
CFLAGS= -O
LDLIBS=
LINT=lint
LINTFLAGS=
SGSBASE=../../cmd/sgs
INS=$(SGSBASE)/sgs.install
STRIP=strip

LIB=$(ROOT)/lib
USRLIB=$(ROOT)/usr/lib
CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib
LORDER=lorder
SOURCES=libmai.c libzer.c
OBJECTS=libmai.o libzer.o
AR=ar
ARFLAGS=rv

all:     $(CCSLIB)/liby.a

$(CCSLIB)/liby.a: $(OBJECTS)
	$(AR) $(ARFLAGS) tmplib.a `$(LORDER) *.o | tsort`;

libmai.o:	libmai.c
		$(CC) -c $(CFLAGS) libmai.c

libzer.o:	libzer.c
		$(CC) -c $(CFLAGS) libzer.c

install:  all
	/bin/sh $(INS) 644 $(OWN) $(GRP) $(CCSLIB)/liby.a tmplib.a;

lintit:	$(SOURCES)
	$(LINT) $(LINTFLAGS) $(SOURCES)

clean:
	-rm -f $(OBJECTS)

clobber:clean
	-rm -f tmplib.a
