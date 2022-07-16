#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#       Portions Copyright(c) 1988, Sun Microsystems, Inc.
#       All Rights Reserved

#ident	"@(#)ucbtset:tset.mk	1.2.3.1"

#     Makefile for tset

ROOT =

DIR = $(ROOT)/usr/ucb

INC = $(ROOT)/usr/include

LDFLAGS =  $(SHLIBS) -s -lcurses -ltermlib 

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size
#top#

MAKEFILE = tset.mk

MAINS = tset 

OBJECTS =  tset.o

SOURCES = tset.c 

ALL:          $(MAINS)

$(MAINS):	tset.o
	$(CC) $(CFLAGS) -o tset tset.o $(LDFLAGS)
	
tset.o:		$(INC)/stdio.h 

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
	install -f $(DIR) -m 00555 -u bin -g bin tset 
	rm -f $(DIR)/reset
	ln $(DIR)/tset $(DIR)/reset
	
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

