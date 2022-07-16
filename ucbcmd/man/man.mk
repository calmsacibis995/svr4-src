#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbman:man.mk	1.3.3.1"


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
#	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#	          All rights reserved.

#	Makefile for man/whatis/apropos/catman

ROOT =

DIR = $(ROOT)/usr/ucb
LIBDIR = $(ROOT)/usr/ucblib

INC = $(ROOT)/usr/include

LDFLAGS = $(SHLIBS) -s

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

LIST = lp

MAKEFILE = man.mk

MAINS = man whatis apropos catman getNAME makewhatis

OBJECTS =  man.o catman.o getNAME.o

SOURCES =  man.c catman.c getNAME.c

ALL:		$(MAINS)

man: man.o
	$(CC) $(CFLAGS)  -o man  man.o $(LDFLAGS)

whatis:	man
	@ln man whatis

apropos: man
	@ln man apropos

catman: catman.o
	$(CC) $(CFLAGS)  -o catman  catman.o $(LDFLAGS)

getNAME: getNAME.o
	$(CC) $(CFLAGS)  -o getNAME  getNAME.o $(LDFLAGS)

makewhatis: makewhatis.sh
	cp makewhatis.sh makewhatis

man.o:	$(INC)/stdio.h $(INC)/ctype.h $(INC)/sys/types.h \
	$(INC)/sys/param.h $(INC)/sgtty.h $(INC)/sys/stat.h \
	$(INC)/signal.h $(INC)/string.h 

catman.o:$(INC)/stdio.h $(INC)/ctype.h $(INC)/sys/types.h \
	$(INC)/sys/param.h $(INC)/errno.h $(INC)/sys/stat.h \
	$(INC)/dirent.h $(INC)/sys/time.h 

getNAME.o:$(INC)/stdio.h $(INC)/string.h 

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

all : ALL

install: ALL
	install -m 555 -u root -g bin -f $(DIR) man 
	-rm -f $(DIR)/whatis $(DIR)/apropos
	ln $(DIR)/man $(DIR)/whatis
	ln $(DIR)/man $(DIR)/apropos
	install -m 555 -u root -g bin -f $(DIR) catman
	install -m 555 -u root -g bin -f $(LIBDIR) getNAME
	install -m 555 -u root -g bin -f $(LIBDIR) makewhatis


size: ALL
	$(SIZE) man catman getNAME

strip: ALL
	$(STRIP) man catman getNAME

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
	pr man.mk $(SOURCE) | $(LIST)

listmk: 
	pr man.mk | $(LIST)
