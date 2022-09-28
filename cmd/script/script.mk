#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#	Portions Copyright (c) 1988, Sun Microsystems, Inc.
# 	All Rights Reserved

#ident	"@(#)script:script.mk	1.3.2.1"

#     Makefile for script

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

# libpt no longer specified. libpt routines moved to libc
LDFLAGS = -s $(SHLIBS)

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

#top#

MAKEFILE = script.mk

MAINS = script

OBJECTS =  script.o

SOURCES = script.c 

ALL:          $(MAINS)

$(MAINS):	script.o
	$(CC) $(CFLAGS) -o script script.o $(LDFLAGS)
	
script.o:	$(INC)/sys/types.h \
		$(INC)/sys/stat.h \
		$(INC)/utmp.h \
		$(INC)/sys/signal.h \
		$(INC)/pwd.h\
		$(INC)/stdio.h \
		$(INC)/ctype.h


GLOBALINCS = 	$(INC)/sys/types.h \
                $(INC)/sys/stat.h \
                $(INC)/utmp.h \
                $(INC)/sys/signal.h \
                $(INC)/pwd.h\
                $(INC)/stdio.h \
                $(INC)/ctype.h

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all :	ALL

install:	ALL
	install -f $(DIR) -m 555 -u bin -g bin script 

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

