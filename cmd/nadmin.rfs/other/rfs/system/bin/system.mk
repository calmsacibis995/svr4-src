#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nadmin.rfs:other/rfs/system/bin/system.mk	1.1.2.1"

ROOT =

TARGETDIR =

INC = $(ROOT)/usr/include

INS = install

LDFLAGS = -s -dy -lnsl

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

MAKEFILE = system.mk

MAINS = getaddr

OBJECTS =  getaddr.o

SOURCES =  getaddr.c

ALL:		$(MAINS)

getaddr:		getaddr.o
	$(CC) $(CFLAGS)  -o getaddr getaddr.o   $(LDFLAGS)

getaddr.o:\
	$(INC)/stdio.h \
	$(INC)/netconfig.h \
	$(INC)/netdir.h \
	$(INC)/tiuser.h

install: $(INS) -m 644 -g bin -u bin -f $(TARGETDIR) getaddr

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all : ALL

size: ALL
	$(SIZE) $(MAINS)

strip: ALL
	$(STRIP) $(MAINS)

