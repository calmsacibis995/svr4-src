#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cpio:cpio.mk	1.1.7.1"

# Makefile for cpio

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

INCSYS = $(ROOT)/usr/include

LDFLAGS = -lgen -lgenIO

CFLAGS = -O 

INS = install

STRIP = strip

SIZE = size

MAKEFILE = Makefile

MAINS = cpio

OBJECTS =  cpio.o cpiostat.o

SOURCES =  cpio.c

ALL:		$(MAINS)

cpio:		$(OBJECTS)	
	$(CC) $(CFLAGS) -o cpio $(OBJECTS) $(LDFLAGS) $(PERFLIBS)


cpio.o:	 cpio.h $(INC)/archives.h $(INC)/stdio.h $(INCSYS)/sys/types.h \
	$(INC)/unistd.h $(INC)/errno.h $(INCSYS)/sys/mkdev.h \
	$(INCSYS)/sys/errno.h $(INC)/fcntl.h $(INC)/memory.h \
	$(INC)/signal.h $(INCSYS)/sys/signal.h $(INC)/varargs.h \
	$(INCSYS)/sys/stat.h $(INC)/utime.h $(INC)/pwd.h 

cpiostat.o:	$(INCSYS)/sys/types.h $(INCSYS)/sys/stat.h

GLOBALINCS = $(INC)/errno.h $(INC)/fcntl.h \
	$(INC)/memory.h $(INC)/pwd.h $(INC)/signal.h \
	$(INC)/stdio.h $(INCSYS)/sys/errno.h \
	$(INCSYS)/sys/signal.h $(INCSYS)/sys/stat.h $(INC)/utime.h \
	$(INCSYS)/sys/types.h $(INC)/varargs.h 


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

all : ALL

install: ALL
	$(INS) -f $(DIR) -m 0555 -u bin -g bin $(MAINS)

size: ALL
	$(SIZE) $(MAINS)

strip: ALL
	$(STRIP) $(MAINS)

#	These targets are useful but optional

partslist:
	@echo $(MAKEFILE) $(SOURCES) $(LOCALINCS)  |  tr ' ' '\012'  |  sort

productdir:
	@echo $(DIR) | tr ' ' '\012' | sort

product:
	@echo $(MAINS)  |  tr ' ' '\012'  | \
	sed 's;^;$(DIR)/;'
