#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)groups:groups.mk	1.6.1.1"

#	Makefile for groups 

ROOT =

DIR = $(ROOT)/usr/bin

INS = install

INC = $(ROOT)/usr/include

LDFLAGS = -s

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

MAKEFILE = groups.mk

MAINS = groups

OBJECTS =  groups.o

SOURCES =  groups.c

ALL:		$(MAINS)

groups:		groups.o	
	$(CC) $(CFLAGS)  -o groups  groups.o   $(LDFLAGS) $(SHLIBS)

groups.o:	 $(INC)/stdio.h $(INC)/grp.h $(INC)/pwd.h $(INC)/sys/sysconfig.h

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
	@fileaudit $(MAKEFILE) $(SOURCES) -o $(OBJECTS) $(MAINS)
