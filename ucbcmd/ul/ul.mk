#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#     Portions Copyright(c) 1988, Sun Microsystems, Inc.
#     All Rights Reserved

#ident	"@(#)ucbul:ul.mk	1.3.1.1"

#     Makefile for ul

ROOT =

DIR = $(ROOT)/usr/ucb

INC = $(ROOT)/usr/include

LDFLAGS = -s $(SHLIBS) -lcurses 

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

#top#

MAKEFILE = ul.mk

MAINS = ul

OBJECTS =  ul.o

SOURCES = ul.c 

ALL:          $(MAINS)

$(MAINS):	ul.o
	$(CC) $(CFLAGS) -o ul ul.o $(LDFLAGS)
	
ul.o:		$(INC)/stdio.h 

GLOBALINCS = $(INC)/stdio.h 

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all :	ALL

install:	ALL
	install -f $(DIR) -u bin -g bin -m 00555 $(MAINS)

size: ALL
	$(SIZE) $(MAINS)

strip: ALL
	$(STRIP) $(MAINS)

#     These targets are useful but optional

partslist:
	@echo $(MAKEFILE) $(SOURCES) $(LOCALINCS)  |  tr ' ' '\012'  |  sort

productdir:
	@echo $(DIR) | tr ' ' '\012' | sort

product:
	@echo $(MAINS)  |  tr ' ' '\012'  | \
	sed 's;^;$(DIR)/;'

srcaudit:
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) -o $(OBJECTS) $(MAINS)

