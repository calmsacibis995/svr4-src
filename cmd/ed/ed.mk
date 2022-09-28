#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ed:ed.mk	1.31"

#	Makefile for ed

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

INS = install

LDFLAGS = -s -lcrypt_i -lgen -lw $(PERFLIBS)

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

#top#

MAKEFILE = ed.mk


MAINS = ed

OBJECTS =  ed.o

SOURCES =  ed.c

ALL:		$(MAINS)

ed:		ed.o 
	$(CC) $(CFLAGS)  -o ed  ed.o   $(LDFLAGS)


ed.o:		 $(INC)/limits.h $(INC)/ctype.h $(INC)/fcntl.h \
		 $(INC)/unistd.h $(INC)/regexpr.h $(INC)/setjmp.h \
		 $(INC)/signal.h $(INC)/stdio.h \
		 $(INC)/stdlib.h $(INC)/sys/fcntl.h	\
		 $(INC)/sys/signal.h $(INC)/sys/stat.h \
		 $(INC)/sys/termio.h $(INC)/sys/types.h \
		 $(INC)/termio.h $(INC)/ustat.h 

GLOBALINCS = $(INC)/limits.h $(INC)/ctype.h $(INC)/fcntl.h \
	$(INC)/regexpr.h $(INC)/setjmp.h $(INC)/signal.h \
	$(INC)/stdio.h $(INC)/stdlib.h $(INC)/sys/fcntl.h \
	$(INC)/sys/signal.h $(INC)/sys/stat.h \
	$(INC)/sys/termio.h $(INC)/sys/types.h \
	$(INC)/termio.h $(INC)/ustat.h 


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
	-rm -f $(DIR)/red
	ln $(DIR)/ed $(DIR)/red

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
