#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbw:w.mk	1.3.3.1"


#		PROPRIETARY NOTICE (Combined)
#
#This source code is unpublished proprietary information
#constituting, or derived under license from AT&T's UNIX(r) System V.
#In addition, portions of such source code were derived from Berkeley
#4.3 BSD under license from the Regents of the University of
#California.
#
#
#
#		Copyright Notice 
#
#Notice of copyright on this source code product does not indicate 
#publication.
#
#	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#	          All rights reserved.

#	Makefile for w/uptime

ROOT =

DIR = $(ROOT)/usr/ucb

INC = $(ROOT)/usr/include

LDFLAGS = $(SHLIBS) -s

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

LIST = lp

MAKEFILE = w.mk

MAINS = w uptime

OBJECTS =  w.o

SOURCES =  w.c

ALL:		$(MAINS)

w: w.o
	$(CC) $(CFLAGS)  -o w  w.o $(LDFLAGS)

uptime:	w
	@/bin/ln w uptime



w.o:	$(INC)/stdio.h $(INC)/fcntl.h $(INC)/time.h $(INC)/sys/types.h \
	$(INC)/sys/types.h $(INC)/sys/param.h $(INC)/utmp.h \
	$(INC)/sys/utsname.h $(INC)/sys/stat.h $(INC)/dirent.h \
	$(INC)/sys/procfs.h $(INC)/sys/proc.h

GLOBALINCS = $(INC)/stdio.h $(INC)/fcntl.h $(INC)/time.h $(INC)/sys/types.h \
        $(INC)/sys/types.h $(INC)/sys/param.h $(INC)/utmp.h \
        $(INC)/sys/utsname.h $(INC)/sys/stat.h $(INC)/dirent.h \
        $(INC)/sys/procfs.h $(INC)/sys/proc.h

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)


all : ALL

install: ALL
	install -m 4555 -u root -g bin -f $(DIR) w 
	rm -f $(DIR)/uptime
	/bin/ln $(DIR)/w $(DIR)/uptime

size: ALL
	$(SIZE) w

strip: ALL
	$(STRIP) w

#	These targets are useful but optional

partslist:
	@echo $(MAKEFILE) $(SOURCES) $(LOCALINCS)  |  tr ' ' '\012'  |  sort

productdir:
	@echo $(DIR) | tr ' ' '\012' | sort

product:
	@echo $(MAINS) |  tr ' ' '\012'  | \
	sed 's;^;$(DIR)/;'

srcaudit:
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) -o $(OBJECTS) $(MAINS)

listing:
	pr w.mk $(SOURCE) | $(LIST)

listmk: 
	pr w.mk | $(LIST)
