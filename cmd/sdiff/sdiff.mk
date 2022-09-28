#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)sdiff:sdiff.mk	1.2.3.1"

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

INS = install

CFLAGS = -O -I$(INC)

LDFLAGS = -s 

STRIP = strip

MAKEFILE = sdiff.mk

MAINS = sdiff

OBJECTS =  sdiff.o

SOURCES =  sdiff.c

all:		$(MAINS)

sdiff:		sdiff.o	
	$(CC) $(CFLAGS)  -o sdiff  sdiff.o   $(LDFLAGS) $(SHLIBS)


sdiff.o:	 $(INC)/stdio.h $(INC)/ctype.h	$(INC)/signal.h \
		 $(INC)/sys/types.h $(INC)/sys/stat.h \
		 $(INC)/unistd.h $(INC)/stdlib.h

clean:
	rm -f $(OBJECTS)
	
clobber:	
	rm -f $(OBJECTS) $(MAINS)

install:	$(MAINS) $(DIR)
	$(INS) -f $(DIR) -m 0555 -u bin -g bin $(MAINS)

strip:
	$(STRIP) $(MAINS)

remove:
	cd $(DIR);  rm -f $(MAINS)

$(DIR):
	mkdir $(DIR);  $(CH)chmod 755 $(DIR);  $(CH)chown bin $(DIR)

partslist:
	@echo $(MAKEFILE) $(LOCALINCS) $(SOURCES)  |  tr ' ' '\012'  |  sort

product:
	@echo $(MAINS)  |  tr ' ' '\012'  | \
		sed -e 's;^;$(DIR)/;' -e 's;//*;/;g'

productdir:
	@echo $(DIR)
