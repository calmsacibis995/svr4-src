#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)cpiopc:cpiopc.mk	1.3"

#	Makefile for cpiopc

ROOT =

DIR = $(ROOT)/usr/sbin

INS = install

INC = $(ROOT)/usr/include

LDFLAGS = -s $(LDLIBS)

CFLAGS = -O -I$(INC) -D_STYPES

STRIP = strip

SIZE = size

#top#

MAKEFILE = Makefile

MAINS = .cpiopc

OBJECTS =  cpiopc.o

SOURCES =  cpiopc.c

ALL:		$(MAINS)

.cpiopc:		cpiopc.o	
	$(CC) $(CFLAGS)  -o .cpiopc  cpiopc.o   $(LDFLAGS)


cpiopc.o:	 $(INC)/stdio.h $(INC)/sys/types.h $(INC)/errno.h \
		 $(INC)/sys/errno.h $(INC)/stdio.h \
		 $(INC)/signal.h $(INC)/sys/signal.h \
		 $(INC)/sys/stat.h $(INC)/pwd.h $(INC)/sys/dir.h

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all : ALL

install: ALL
	$(INS) -f $(DIR) -u root -m 0755 -g sys $(MAINS)

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
