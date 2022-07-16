#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:cmd/mt/mt.mk	1.3.1.3"

DIR = $(ROOT)/sbin
TDIR = $(ROOT)/usr/lib/tape

INC=$(ROOT)/usr/include

LDFLAGS = -s

CFLAGS= -O -I$(INC)

STRIP = strip

SIZE = size

MAKEFILE = mt.mk

MAINS = mt

OBJECTS =  mt.o

SOURCES =  mt.c

ALL:		$(MAINS)

mt.o:	mt.c
	$(CC) $(CFLAGS) -c mt.c

mt:	mt.o
	$(CC) $(LDFLAGS) -o ./mt mt.o $(SHLIBS)

mt.c:            \
		 $(INC)/sys/types.h \
		 $(INC)/sys/param.h \
		 $(INC)/sys/tape.h \
		 $(INC)/stdio.h \
		 $(INC)/errno.h

clean:
	rm -f mt.o

clobber:        clean
	rm -f $(MAINS) tapecntl

all : ALL

install: ALL
	install -f $(DIR) -m 0711 -u bin -g bin $(MAINS) 
	cp mt tapecntl
	[ -d $(TDIR) ] || mkdir $(TDIR)
	install -f $(TDIR) -m 0711 -u bin -g bin tapecntl
