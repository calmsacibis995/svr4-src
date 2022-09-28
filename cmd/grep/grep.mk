#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)grep:grep.mk	1.6.3.1"

#	Makefile for grep 

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

LDFLAGS = -s -lgen $(ROOTLIBS)

CFLAGS = -O -I$(INC)

INS = install

STRIP = strip

SIZE = size

MAKEFILE = grep.mk


MAINS = grep

OBJECTS =  grep.o

SOURCES =  grep.c

ALL:		$(MAINS)

grep:		grep.o 
	$(CC) $(CFLAGS)  -o grep  grep.o   $(LDFLAGS)


grep.o:		 $(INC)/ctype.h $(INC)/memory.h \
		 $(INC)/regexpr.h	$(INC)/stdio.h 

GLOBALINCS = $(INC)/ctype.h $(INC)/memory.h \
	$(INC)/regexpr.h $(INC)/stdio.h 


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
	$(INS) -f $(ROOT)/sbin -m 0555 -u bin -g bin $(MAINS) 

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
