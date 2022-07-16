#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#     Portions Copyright(c) 1988, Sun Microsystems, Inc.
#     All Rights Reserved

#ident	"@(#)ucblastcomm:lastcomm.mk	1.3.1.1"

#     Makefile for lastcomm

ROOT =

DIR = $(ROOT)/usr/ucb

INC = $(ROOT)/usr/include

LDFLAGS = -s $(SHLIBS) 

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

#top#

MAKEFILE = lastcomm.mk

MAINS = lastcomm

OBJECTS =  lastcomm.o

SOURCES = lastcomm.c 

ALL:          $(MAINS)

$(MAINS):	lastcomm.o
	$(CC) $(CFLAGS) -o lastcomm lastcomm.o $(LDFLAGS)
	
lastcomm.o:		$(INC)/stdio.h \
		$(INC)/sys/types.h \
		$(INC)/pwd.h \
		$(INC)/sys/stat.h \
		$(INC)/utmp.h \
		$(INC)/ctype.h \
		$(INC)/sys/param.h \
		$(INC)/sys/acct.h \
		$(INC)/sys/file.h 


GLOBALINCS = $(INC)/stdio.h \
		$(INC)/sys/types.h \
		$(INC)/pwd.h \
		$(INC)/sys/stat.h \
		$(INC)/utmp.h \
		$(INC)/ctype.h \
		$(INC)/sys/param.h \
		$(INC)/sys/acct.h \
		$(INC)/sys/file.h 


clean:
	rm -f $(OBJECTS)

clobber:	clean
	rm -f $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all :	ALL

install:	ALL
	install -f $(DIR) -m 00555 -u bin -g bin lastcomm 

size: ALL
	$(SIZE) $(MAINS)

strip: ALL
	$(STRIP) $(MAINS)

#     These targets are useful but optional

partslist:
	@echo $(MAKEFILE) $(SOURCES) $(LOCALINCS)  |  tr ' ' '\012'  |  sort

productdir:
	@echo $(DIR) | tr ' ' '\012' | sort

product:
	@echo $(MAINS)  |  tr ' ' '\012'  | \
	sed 's;^;$(DIR)/;'

srcaudit:
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) -o $(OBJECTS) $(MAINS)

