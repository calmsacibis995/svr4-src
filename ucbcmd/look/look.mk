#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#     Portions Copyright(c) 1988, Sun Microsystems, Inc.
#     All Rights Reserved

#ident	"@(#)ucblook:look.mk	1.3.1.1"

#     Makefile for look

ROOT =

DIR = $(ROOT)/usr/ucb

LIBDIR = $(ROOT)/usr/ucblib

INC = $(ROOT)/usr/include

LDFLAGS = -s $(SHLIBS) 

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

#top#

MAKEFILE = look.mk

MAINS = look

OBJECTS =  look.o

SOURCES = look.c 

ALL:          $(MAINS)

$(MAINS):	look.o
	$(CC) $(CFLAGS) -o look look.o $(LDFLAGS)
	
look.o:		$(INC)/stdio.h \
		$(INC)/ctype.h 

GLOBALINCS = $(INC)/stdio.h \
	$(INC)/ctype.h 

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
	- mkdir $(LIBDIR)/dict
	$(CH)-chown bin $(LIBDIR)/dict
	$(CH)-chgrp bin $(LIBDIR)/dict
	$(CH)-chmod 755 $(LIBDIR)/dict
	install -f $(LIBDIR)/dict -u bin -g bin -m 0444 words

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

