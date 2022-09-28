#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ls:ls.mk	1.13.9.3"

#	Makefile for ls

ROOT =

DIR = $(ROOT)/usr/bin


INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include

LDFLAGS = -s -lgen

INS = install

CFLAGS = -O -I$(INC) -I$(INCSYS)

STRIP = strip

SIZE = size

INS = install

#top#

MAKEFILE = ls.mk

MAINS = ls

OBJECTS =  ls.o

SOURCES =  ls.c

ALL:		$(MAINS)

ls:	ls.o
	$(CC) $(CFLAGS) -o ls ls.o $(LDFLAGS) $(PERFLIBS)

ls.o:		 $(INCSYS)/sys/param.h \
		 $(INCSYS)/sys/types.h \
		 $(INCSYS)/sys/sysmacros.h \
		 $(INCSYS)/sys/stat.h \
		 $(INC)/stdio.h \
		 $(INC)/dirent.h \
		 $(INCSYS)/sys/dirent.h \
		 $(INC)/stdio.h \
		 $(INC)/unctrl.h	\
		 $(INC)/termio.h	\
		 $(INCSYS)/sys/termio.h \
		 $(INC)/term.h 

GLOBALINCS = $(INC)/stdio.h \
	$(INC)/curses.h \
	$(INC)/dirent.h \
	$(INCSYS)/sys/dirent.h \
	$(INCSYS)/sys/param.h \
	$(INCSYS)/sys/stat.h \
	$(INCSYS)/sys/sysmacros.h \
	$(INCSYS)/sys/termio.h \
	$(INCSYS)/sys/types.h \
	$(INC)/term.h \
	$(INC)/termio.h \
	$(INC)/unctrl.h 


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

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

srcaudit:
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) -o $(OBJECTS) $(MAINS)
