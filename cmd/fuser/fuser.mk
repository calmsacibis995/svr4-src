#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fuser:fuser.mk	1.1.8.1"



#	Makefile for fuser

ROOT =
DIR = $(ROOT)/usr/sbin
INC = $(ROOT)/usr/include
LDFLAGS = -s -lelf $(SHLIBS)
SYMLINK = :
CFLAGS = -O -I$(INC)
STRIP = strip
SIZE = size
INS=install
INSDIR= $(ROOT)/usr/sbin

MAKEFILE = fuser.mk

MAINS = fuser

OBJECTS =  fuser.o

SOURCES =  fuser.c

ALL:		$(MAINS)

$(MAINS):	fuser.o
		$(CC) $(CFLAGS) -o fuser fuser.o $(LDFLAGS)

fuser.o:	$(INC)/nlist.h \
		$(INC)/fcntl.h \
		$(INC)/pwd.h \
		$(INC)/stdio.h \
		$(INC)/sys/mnttab.h \
		$(INC)/sys/errno.h \
		$(INC)/sys/param.h \
		$(INC)/sys/var.h \
		$(INC)/sys/utssys.h

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) fuser

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all : ALL

install: ALL
	-rm -f $(ROOT)/etc/fuser
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin fuser
	-$(SYMLINK) /usr/sbin/fuser $(ROOT)/etc/fuser

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
