#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)mv:mv.mk	1.16.10.4"

#	Makefile for mv/cp/ln

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

LDFLAGS = -s $(NOSHLIBS)

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

LIST = lp

MAKEFILE = mv.mk

MAINS = mv ln cp

INS = install

OBJECTS =  mv.o

SOURCES =  mv.c

ALL:		$(MAINS)

compile: mv.o
	$(CC) $(CFLAGS)  -o ln  mv.o $(LDFLAGS)

cp:	compile
	@/bin/ln ln cp

mv:	compile
	@/bin/ln ln mv

ln:	compile 


mv.o:		 $(INC)/stdio.h $(INC)/sys/types.h \
		 $(INC)/sys/stat.h $(INC)/fcntl.h \
		 $(INC)/utime.h $(INC)/signal.h \
		 $(INC)/errno.h $(INC)/sys/errno.h \
		 $(INC)/sys/param.h $(INC)/dirent.h \
		 $(INC)/sys/dir.h $(INC)/stdlib.h 

GLOBALINCS =	 $(INC)/stdio.h $(INC)/sys/types.h \
		 $(INC)/sys/stat.h $(INC)/fcntl.h \
		 $(INC)/utime.h $(INC)/signal.h \
		 $(INC)/errno.h $(INC)/sys/errno.h \
		 $(INC)/sys/param.h $(INC)/dirent.h \
		 $(INC)/sys/dir.h $(INC)/stdlib.h 


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS) $(ROOT)/bin/OLDln


all : ALL

install: ALL
	$(INS) -o -m 0555 -u bin -g bin -f $(DIR) ln
	/bin/ln $(DIR)/ln $(DIR)/mv
	/bin/ln $(DIR)/ln $(DIR)/cp

size: ALL
	$(SIZE) ln

strip: ALL
	$(STRIP) ln

#	These targets are useful but optional

partslist:
	@echo $(MAKEFILE) $(SOURCES) $(LOCALINCS)  |  tr ' ' '\012'  |  sort

productdir:
	@echo $(DIR) | tr ' ' '\012' | sort

product:
	@echo $(MAINS) |  tr ' ' '\012'  | \
	sed 's;^;$(DIR)/;'

srcaudit:
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) -o $(OBJECTS) $(MAINS)

listing:
	pr mv.mk $(SOURCE) | $(LIST)

listmk: 
	pr mv.mk | $(LIST)
