#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)ttymon:stty.mk	1.13.3.1"

#	Makefile for stty 

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include

LDFLAGS = -s

DFLGS = -DMERGE386

CFLAGS = -O -I$(INC) -I$(INCSYS) $(DFLGS)

STRIP = strip

SIZE = size

MAKEFILE = stty.mk

MAINS = stty

OBJECTS =  stty.o sttytable.o sttyparse.o

SOURCES =  stty.c sttytable.c sttyparse.c

ALL:		$(MAINS)

stty:	$(OBJECTS)
	$(CC) $(CFLAGS) -o stty $(OBJECTS) $(LDFLAGS) $(NOSHLIBS)

stty.o:		 stty.c \
		 stty.h \
		 $(INC)/stdio.h \
		 $(INCSYS)/sys/types.h \
		 $(INCSYS)/sys/termio.h \
		 $(INCSYS)/sys/stermio.h \
		 $(INCSYS)/sys/termiox.h 

sttytable.o:	 sttytable.c \
		 stty.h \
		 $(INC)/stdio.h \
		 $(INCSYS)/sys/types.h \
		 $(INCSYS)/sys/termio.h \
		 $(INCSYS)/sys/stermio.h \
		 $(INCSYS)/sys/termiox.h 

sttyparse.o:	 sttyparse.c \
		 stty.h \
		 $(INC)/stdio.h \
		 $(INC)/unistd.h \
		 $(INC)/stdlib.h \
		 $(INCSYS)/sys/types.h \
		 $(INCSYS)/sys/termio.h \
		 $(INCSYS)/sys/stermio.h \
		 $(INCSYS)/sys/termiox.h 

GLOBALINCS = $(INC)/stdio.h \
	$(INCSYS)/sys/stermio.h \
	$(INCSYS)/sys/termio.h \
	$(INCSYS)/sys/termiox.h \
	$(INCSYS)/sys/types.h 


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all : ALL

install: ALL
	install -f $(DIR) $(MAINS)

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
