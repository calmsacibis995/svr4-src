#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)exstr:exstr.mk	1.3.3.1"

#	Makefile for exstr

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

LDFLAGS = -s $(SHLIBS)

CFLAGS = -O -I$(INC) 

STRIP = strip

SIZE = size

INS = install

MAKEFILE = exstr.mk


MAINS = exstr

OBJECTS =  exstr.o

SOURCES =  exstr.c

ALL:		$(MAINS)

exstr:		exstr.o 
	$(CC) $(CFLAGS)  -o exstr  exstr.o   $(LDFLAGS)


exstr.o:		 $(INC)/stdio.h $(INC)/sys/types.h \
		 $(INC)/sys/stat.h 

GLOBALINCS = $(INC)/stdio.h $(INC)/vargs.h $(INC)/string.h \
	$(INC)/sys/signal.h 


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
