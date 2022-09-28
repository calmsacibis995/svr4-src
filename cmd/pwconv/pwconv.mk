#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)pwconv:pwconv.mk	1.8.3.1"

ROOT =

DIR = $(ROOT)/usr/sbin

INC = $(ROOT)/usr/include
SYMLINK = :
INS = install

CFLAGS = -O -I$(INC)

#	Common Libraries and -l<lib> flags.
LDFLAGS = -s $(SHLIBS)

STRIP = strip

MAKEFILE = pwconv.mk

MAINS = pwconv

OBJECTS =  pwconv.o

SOURCES =  pwconv.c

all:		$(MAINS)

pwconv:		pwconv.o	
	$(CC) $(CFLAGS)  -o pwconv  pwconv.o   $(LDFLAGS)


pwconv.o:	 $(INC)/pwd.h $(INC)/fcntl.h $(INC)/sys/fcntl.h \
		 $(INC)/stdio.h $(INC)/sys/types.h $(INC)/sys/select.h \
		 $(INC)/sys/stat.h $(INC)/time.h	\
		 $(INC)/shadow.h $(INC)/grp.h $(INC)/signal.h \
		 $(INC)/sys/signal.h $(INC)/errno.h $(INC)/sys/errno.h \
		 $(INC)/unistd.h $(INC)/stdlib.h $(INC)/limits.h

clean:
	rm -f $(OBJECTS)
	
clobber:	
	rm -f $(OBJECTS) $(MAINS)

install:	$(MAINS) $(DIR)
	-rm -f $(ROOT)/usr/bin/pwconv
	$(INS) -f $(DIR) -m 0500 -u root -g sys $(MAINS)
	-$(SYMLINK) /usr/sbin/pwconv $(ROOT)/usr/bin/pwconv

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
