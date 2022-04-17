#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)backup:backup.mk	1.1"

#	Makefile for backup

ROOT =

DIR = $(ROOT)/usr/bin
IDIR = $(ROOT)/etc

INC = $(ROOT)/usr/include

LDFLAGS = -s

CFLAGS = -O -I$(INC)

INSTALL = install

STRIP = strip

SIZE = size

MAINS = pwdmenu backup

OBJECTS =  pwdmenu.o

SOURCES =  pwdmenu.c backup.sh

IDENT = \
	Backup \
	Ignore

ALL:		$(MAINS)

pwdmenu:	pwdmenu.o 
	$(CC) $(CFLAGS)  -o pwdmenu  pwdmenu.o   $(LDFLAGS)

pwdmenu.o:	 $(INC)/stdio.h $(INC)/pwd.h \
		 $(INC)/string.h 

GLOBALINCS = $(INC)/pwd.h $(INC)/stdio.h $(INC)/string.h 

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

all : ALL

install: ALL
	for i in $(IDENT); \
	do \
		grep -v "^#" $$i.sh | grep -v "^$$" > $(IDIR)/$$i ;\
	done \

	for i in $(MAINS); do \
		$(INSTALL) -f $(DIR) $$i; \
	done
	-ln $(DIR)/backup $(DIR)/.backup

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
