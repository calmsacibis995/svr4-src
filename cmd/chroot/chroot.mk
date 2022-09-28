#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)chroot:chroot.mk	1.9.3.1"
#	Makefile for chroot

ROOT =

DIR = $(ROOT)/usr/sbin

SYMLINK = :

INC = $(ROOT)/usr/include

LDFLAGS = -s

CFLAGS = -O -I$(INC)

INS = install

STRIP = strip

SIZE = size

#top#

MAKEFILE = chroot.mk

MAINS = chroot

OBJECTS =  chroot.o

SOURCES =  chroot.c

ALL:		$(MAINS)

$(MAINS):	chroot.o
	$(CC) $(CFLAGS) -o chroot chroot.o $(LDFLAGS) $(NOSHLIBS)

chroot.o:	 $(INC)/stdio.h $(INC)/errno.h \
		 $(INC)/sys/errno.h $(INC)/unistd.h 

GLOBALINCS =	 $(INC)/stdio.h  $(INC)/errno.h \
		 $(INC)/sys/errno.h $(INC)/unistd.h 


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all : ALL

install: ALL 
	-rm -f $(ROOT)/etc/chroot
	$(INS) -f $(DIR) -m 0555 -u bin -g bin $(MAINS)
	-$(SYMLINK) /usr/sbin/chroot $(ROOT)/etc/chroot

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



