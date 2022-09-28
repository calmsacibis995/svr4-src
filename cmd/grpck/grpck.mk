#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)grpck:grpck.mk	1.5.3.1"

#	Makefile for grpck

ROOT =

DIR = $(ROOT)/usr/sbin

INC = $(ROOT)/usr/include

LDFLAGS = -s $(SHLIBS)

SYMLINK = :

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

INS = install

#top#

MAKEFILE = grpck.mk

MAINS = grpck

OBJECTS =  grpck.o

SOURCES =  grpck.c

ALL:		$(MAINS)

$(MAINS):	$(OBJECTS)
		$(CC) $(CFLAGS) -o grpck $(OBJECTS) $(LDFLAGS)

grpck.o:	 $(INC)/stdio.h \
		 $(INC)/sys/types.h \
		 $(INC)/ctype.h \
		 $(INC)/pwd.h 

GLOBALINCS = $(INC)/ctype.h \
	$(INC)/pwd.h \
	$(INC)/stdio.h \
	$(INC)/sys/types.h


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all : ALL

install: ALL
	-rm -f $(ROOT)/etc/grpck
	$(INS) -f $(DIR) -m 0555 -u bin -g bin $(MAINS) 
	-$(SYMLINK) /usr/sbin/grpck $(ROOT)/etc/grpck

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



