#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)od:od.mk	1.4.1.1"

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

INS = install

CFLAGS = -O -I$(INC)

LDFLAGS = -s 

STRIP = strip

MAKEFILE = od.mk

MAINS = od

OBJECTS =  od.o

SOURCES =  od.c

all:		$(MAINS)

od:		od.o	
	$(CC) $(CFLAGS)  -o od  od.o   $(LDFLAGS) $(SHLIBS)


od.o:	 $(INC)/stdio.h $(INC)/stdlib.h	$(INC)/ctype.h 

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
