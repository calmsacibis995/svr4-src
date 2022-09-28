#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)pack:pack.mk	1.10.1.1"
#	Makefile for pack

ROOT =
DIR = $(ROOT)/usr/bin
INC = $(ROOT)/usr/include
LDFLAGS = -s $(PERFLIBS)
CFLAGS = -O -I$(INC) $(FFLAG)
STRIP = strip
SIZE = size
MAKEFILE = pack.mk
MAINS = pack
OBJECTS =  pack.o
SOURCES =  pack.c

ALL:		$(MAINS)

pack:		pack.o 
	$(CC) $(CFLAGS)  -o pack  pack.o   $(LDFLAGS)


pack.o:		 $(INC)/stdio.h $(INC)/sys/types.h \
		 $(INC)/sys/stat.h $(INC)/unistd.h

GLOBALINCS = $(INC)/stdio.h $(INC)/sys/stat.h \
	$(INC)/sys/types.h $(INC)/unistd.h


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all : ALL

install: ALL
	install -f $(DIR) -m 00555 -u bin -g bin pack

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
	@fileaudit $(MAKEFILE) $(GLOBALINCS) $(SOURCES) -o $(OBJECTS) $(MAINS)
