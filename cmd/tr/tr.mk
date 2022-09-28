#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)tr:tr.mk	1.3.1.1"

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

INS = install

CFLAGS = -O -I$(INC)

#	Common Libraries and -l<lib> flags.
LDFLAGS = -s $(SHLIBS)

STRIP = strip

MAKEFILE = tr.mk

MAINS = tr

OBJECTS =  tr.o

SOURCES =  tr.c

all:	$(MAINS)

tr:	tr.o	
	$(CC) $(CFLAGS)  -o tr  tr.o   $(LDFLAGS)


tr.o:	 $(INC)/stdio.h 

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
	mkdir $(DIR);  $(CH)chmod 775 $(DIR);  $(CH)chown bin $(DIR); $(CH)chgrp bin $(DIR)

partslist:
	@echo $(MAKEFILE) $(LOCALINCS) $(SOURCES)  |  tr ' ' '\012'  |  sort

product:
	@echo $(MAINS)  |  tr ' ' '\012'  | \
		sed -e 's;^;$(DIR)/;' -e 's;//*;/;g'

productdir:
	@echo $(DIR)
