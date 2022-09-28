#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)sed:sed.mk	1.16"
#	Makefile for sed

ROOT =

DIR = $(ROOT)/usr/bin

SYMLINK = :

INC = $(ROOT)/usr/include

LDFLAGS = -s -lgen -lw

CFLAGS = -O -I$(INC)

INS = install

STRIP = strip

SIZE = size

MAKEFILE = sed.mk


MAINS = sed

OBJECTS =  sed0.o sed1.o

SOURCES =  sed0.c sed1.c

ALL:		$(MAINS)

sed:		sed0.o sed1.o 
	$(CC) $(CFLAGS)  -o sed  sed0.o sed1.o $(LDFLAGS) $(NOSHLIBS)


sed0.o:		 $(INC)/regexpr.h	$(INC)/stdio.h \
		 $(INC)/stdlib.h $(INC)/limits.h sed.h 


sed1.o:		 $(INC)/ctype.h $(INC)/regexpr.h \
		 $(INC)/stdio.h $(INC)/stdlib.h $(INC)/limits.h sed.h 

GLOBALINCS = $(INC)/ctype.h $(INC)/regexpr.h \
	$(INC)/stdio.h $(INC)/stdlib.h $(INC)/limits.h

LOCALINCS = sed.h 

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
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) -o $(OBJECTS) $(MAINS)
