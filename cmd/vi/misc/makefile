#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)vi:misc/makefile	1.20"

#	Makefile for misc directory for vi

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

LDFLAGS = -s $(PERFLIBS)

NLDFLAGS = -s

CFLAGS = -O -I$(INC)

INS = install

STRIP = strip

SIZE = size

MAKEFILE = makefile

MAINS = ctags fold cxref

OBJECTS =  ctags.o fold.o 

SOURCES =  ctags.c fold.c cxref.sh

ALL:		$(MAINS)

ctags:		ctags.o	
	$(CC) $(CFLAGS)  -o ctags  ctags.o   $(LDFLAGS)

fold:		fold.o 
	$(CC) $(CFLAGS)  -o fold  fold.o   $(LDFLAGS)


ctags.o:	 $(INC)/stdio.h $(INC)/ctype.h 

fold.o:		 $(INC)/stdio.h 

GLOBALINCS = $(INC)/ctype.h $(INC)/signal.h $(INC)/stdio.h \
	$(INC)/sys/signal.h $(INC)/sys/stat.h \
	$(INC)/sys/types.h 


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

all : ALL

install: ALL
	$(INS) -f $(DIR) -m 555 -u bin -g bin ctags 

size: ctags
	$(SIZE) ctags

strip: ALL
	$(STRIP) $(MAINS)


partslist:
	@echo $(MAKEFILE) $(SOURCES) $(LOCALINCS)  |  tr ' ' '\012'  |  sort

productdir:
	@echo $(DIR) | tr ' ' '\012' | sort

product:
	@echo ctags  |  tr ' ' '\012' | \
	sed 's;^;$(DIR)/;'

srcaudit:
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) READ_ME del.h.cb -o $(OBJECTS) $(MAINS)
