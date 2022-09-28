#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#	Portions Copyright (c) 1988, Sun Microsystems, Inc.
#	All Rights Reserved.

#ident	"@(#)id:id.mk	1.4.2.1"

#	Makefile for id

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

LDFLAGS = -s $(SHLIBS)

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

INS = install

#top#

MAKEFILE = id.mk

MAINS = id

OBJECTS =  id.o

SOURCES =  id.c

ALL:		$(MAINS)

id:	id.o
	$(CC) $(CFLAGS) -o id id.o $(LDFLAGS)

id.o:		 $(INC)/stdio.h \
		 $(INC)/pwd.h \
		 $(INC)/grp.h \
		 $(INC)/sys/param.h

GLOBALINCS = $(INC)/grp.h \
	$(INC)/pwd.h \
	$(INC)/stdio.h \
	$(INC)/sys/param.h


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all : ALL

install: ALL
	$(INS) -f $(DIR) -m 00555 -u bin -g bin id

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
