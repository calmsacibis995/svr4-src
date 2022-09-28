#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)passmgmt:passmgmt.mk	1.9.1.1"

ROOT =

DIR = $(ROOT)/usr/sbin

DIR1 = $(ROOT)/etc

INC = $(ROOT)/usr/include

SYMLINK = :

INS = "install"

CFLAGS = -O -I$(INC)

#	Common Libraries and -l<lib> flags.
LDFLAGS = -s -lgen $(SHLIBS)

STRIP = strip

MAKEFILE = passmgmt.mk

MAINS = passmgmt

OBJECTS =  passmgmt.o

SOURCES =  passmgmt.c
 
DATEFILE = datemsk

ALL:		$(MAINS)

passmgmt:		passmgmt.o	
	$(CC) $(CFLAGS)  -o passmgmt  passmgmt.o   $(LDFLAGS)


passmgmt.o:	$(INC)/stdio.h $(INC)/sys/types.h $(INC)/shadow.h \
		 $(INC)/pwd.h $(INC)/string.h $(INC)/signal.h \
		 $(INC)/sys/stat.h $(INC)/errno.h $(INC)/sys/errno.h \
		 $(INC)/time.h $(INC)/unistd.h $(INC)/stdlib.h

clean:
	rm -f $(OBJECTS)
	
clobber:	
	rm -f $(OBJECTS) $(MAINS)

install:	$(MAINS) $(DIR)
	$(INS) -f $(DIR) -m 0500 -u root -g sys $(MAINS)
	-$(SYMLINK) /usr/sbin/passmgmt $(ROOT)/usr/bin/passmgmt

	$(INS) -f $(DIR1) -m 0444 -u root -g sys $(DATEFILE)

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
