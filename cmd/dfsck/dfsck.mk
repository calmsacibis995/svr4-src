#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)dfsck:dfsck.mk	1.4.2.1"

#	Makefile for dfsck

ROOT =

DIR = $(ROOT)/usr/sbin

SYMLINK = :
INS = install

INC = $(ROOT)/usr/include

LDFLAGS = -s $(SHLIBS)

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

#top#

MAKEFILE = dfsck.mk

MAINS = dfsck

OBJECTS =  dfsck.o

SOURCES =  dfsck.c

ALL:		$(MAINS)

$(MAINS):	dfsck.o
		$(CC) $(CFLAGS) -o dfsck dfsck.o $(LDFLAGS)

dfsck.o:	 $(INC)/stdio.h \
		 $(INC)/fcntl.h \
		 $(INC)/signal.h \
		 $(INC)/sys/signal.h \
		 $(INC)/errno.h \
		 $(INC)/sys/errno.h \
		 $(INC)/sys/types.h \
		 $(INC)/sys/stat.h 

GLOBALINCS = $(INC)/errno.h \
	$(INC)/fcntl.h \
	$(INC)/signal.h \
	$(INC)/stdio.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/stat.h \
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
	-rm -f $(ROOT)/etc/dfsck
	$(INS) -f $(DIR) -m 0555 -u bin -g bin $(MAINS)
	-$(SYMLINK) /usr/sbin/dfsck $(ROOT)/etc/dfsck

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



