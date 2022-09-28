#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mkpart:mkpart.mk	1.1"

ROOT =

DIR = $(ROOT)/sbin

INC = $(ROOT)/usr/include

LDFLAGS = -s $(ROOTLIBS)

STRIP = strip

SIZE = size

CMDNAME = mkpart

BUS = AT386
ARCH=AT386
LFLAGS = -lelf
IFLAGS = -I$(INC)
CFLAGS = -O $(DEBUG) $(IFLAGS)  -D$(BUS) -D$(ARCH)
YFLAGS = -vd
OFILES = mkpart.o scan.o y.tab.o
YFILES = y.output y.tab.c y.tab.h

all:    $(CMDNAME)

mkpart:         $(OFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o mkpart $(OFILES) $(LFLAGS)

y.tab.o:        partitions.y mkpart.h parse.h
	$(YACC) $(YFLAGS) partitions.y
	$(CC) $(CFLAGS) -c y.tab.c

y.tab.h:        y.tab.o

scan.o:         scan.c y.tab.h mkpart.h parse.h
	$(CC) $(CFLAGS) -c  scan.c

mkpart.o:       mkpart.c \
		y.tab.h \
		mkpart.h \
		parse.h \
		$(INC)/sys/types.h \
		$(INC)/sys/stat.h \
		$(INC)/sys/fs/s5param.h \
		$(INC)/a.out.h \
		$(INC)/fcntl.h \
		$(INC)/sys/vtoc.h \
		$(INC)/sys/sdi_edt.h \
		$(INC)/sys/alttbl.h
	$(CC) $(CFLAGS) -c mkpart.c

clean:
	rm -f $(OFILES) $(YFILES)

clobber: clean
	rm -f $(CMDNAME)

install:        all
		cpset $(CMDNAME) $(DIR)

size:           all
		$(SIZE) $(CMDNAME)

strip:          all
		$(STRIP) $(CMDNAME)
