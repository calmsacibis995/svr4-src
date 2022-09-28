#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rconsole:rconsole.mk	1.1"

ROOT =

INSDIR = $(ROOT)/sbin

INC = $(ROOT)/usr/include

LDFLAGS = -s 

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

MAKEFILE = rconsole.mk

MAINS = conflgs chkconsole isat386

OBJECTS =  conflgs.o chkconsole.o isat386.o

SOURCES =  conflgs.c chkconsole.c isat386.c

ALL:		$(MAINS)

conflgs:	conflgs.o 
	$(CC) $(CFLAGS) -o conflgs conflgs.o $(LDFLAGS) $(LDLIBS) $(SHLIBS)

chkconsole:	chkconsole.o 
	$(CC) $(CFLAGS) -o chkconsole chkconsole.o $(LDFLAGS) $(ROOTLIBS)

isat386:	isat386.o 
	$(CC) $(CFLAGS) -o isat386 isat386.o $(LDFLAGS) $(LDLIBS) $(SHLIBS)


conflgs.o:	$(INC)/stdio.h \
		$(INC)/sys/sysmsg.h \
		$(INC)/sys/types.h \
		$(INC)/errno.h \
		$(INC)/fcntl.h \
		$(INC)/sys/sysi86.h

chkconsole.o:	$(INC)/stdio.h \
		$(INC)/sys/types.h \
		$(INC)/errno.h \
		$(INC)/fcntl.h \
		$(INC)/sys/stat.h \
		$(INC)/sys/sysmsg.h \
		$(INC)/sys/sysmacros.h

isat386.o:	$(INC)/stdio.h \
		$(INC)/sys/types.h \
		$(INC)/errno.h \
		$(INC)/sys/at_ansi.h \
		$(INC)/sys/kd.h

clean:
	-rm -f $(OBJECTS)

clobber: clean
	-rm -f $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE) -s INC $(INC)

all : ALL

install: ALL
	install -f $(INSDIR) chkconsole
	install -f $(INSDIR) conflgs
	install -f $(INSDIR) isat386

size: ALL
	$(SIZE) $(MAINS)

strip: ALL
	$(STRIP) $(MAINS)
