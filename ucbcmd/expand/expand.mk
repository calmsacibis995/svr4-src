#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbexpand:expand.mk	1.1.1.1"

#		PROPRIETARY NOTICE (Combined)
#
#This source code is unpublished proprietary information
#constituting, or derived under license from AT&T's UNIX(r) System V.
#In addition, portions of such source code were derived from Berkeley
#4.3 BSD under license from the Regents of the University of
#California.
#
#
#
#		Copyright Notice 
#
#Notice of copyright on this source code product does not indicate 
#publication.
#
#	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#	          All rights reserved.

#	Makefile for expand and unexpand

ROOT =

DIR = $(ROOT)/usr/ucb

SYMLINK = :

INC = $(ROOT)/usr/include

LDFLAGS = -s $(PERFLIBS)

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

INSTALL = install

#top#
# Generated by makefile 1.47

MAKEFILE = expand.mk


MAINS = expand unexpand

OBJECTS =  expand.o unexpand.o

SOURCES =  expand.c unexpand.c

ALL:		$(MAINS)

expand:		expand.o 
	$(CC) $(CFLAGS)  -o expand  expand.o   $(LDFLAGS)

expand.o:	 $(INC)/stdio.h

unexpand:	unexpand.o
	$(CC) $(CFLAGS)  -o unexpand  unexpand.o   $(LDFLAGS)

unexpand.o:	$(INC)/stdio.h

GLOBALINCS = $(INC)/stdio.h


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all : ALL

install: ALL
	install -u bin -g bin -m 0555 -f $(DIR) expand
	install -u bin -g bin -m 0555 -f $(DIR) unexpand

size: ALL
	$(SIZE) expand
	$(SIZE) unexpand

strip: ALL
	$(STRIP) expand
	$(STRIP) unexpand

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