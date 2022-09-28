#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)newgrp:newgrp.mk	1.6.2.1"

#	Makefile for <newgrp>

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

LDFLAGS = -lcrypt_i $(SHLIBS)

CFLAGS = -O -s -I$(INC)

STRIP = strip

SIZE = size

MAKEFILE = newgrp.mk

INS = install

MAINS = newgrp

OBJECTS =  newgrp.o

SOURCES =  newgrp.c

ALL:		$(MAINS)

newgrp:		newgrp.o 
	$(CC) $(CFLAGS)  -o newgrp  newgrp.o   $(LDFLAGS)


newgrp.o:	 $(INC)/stdio.h $(INC)/sys/types.h $(INC)/pwd.h \
		 $(INC)/grp.h $(INC)/crypt.h $(INC)/string.h \
		 $(INC)/stdlib.h

GLOBALINCS =	 $(INC)/stdio.h $(INC)/sys/types.h $(INC)/pwd.h \
		 $(INC)/grp.h $(INC)/crypt.h $(INC)/string.h \
		 $(INC)/stdlib.h


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all : ALL

install: ALL
	$(INS) -f $(DIR) -m 04755  -u root -g sys newgrp

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
