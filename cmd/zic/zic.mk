#ident	"@(#)zic.mk	1.2	91/09/14	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)zic:zic.mk	1.1.1.1"


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

# Makefile for zic

# If you want something other than Eastern United States time used on your
# system, change the line below (after finding the zone you want in the
# time zone files, or adding it to a time zone file).
# Alternately, if you discover you've got the wrong time zone, you can just
#	zic -l rightzone


################################################################################

ROOT =

DIR = $(ROOT)/usr/sbin

INSLIB = $(ROOT)/usr/lib

INC = $(ROOT)/usr/include

LDFLAGS = -s $(LDLIBS)

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

#top#

CPPFLAGS = -DNOSOLAR

MAKEFILE = zic.mk

MAINS = zic

OBJECTS = zic.o scheck.o ialloc.o

SOURCES = zic.c scheck.c ialloc.c

FILES = asia australasia europe etcetera northamerica pacificnew \
	southamerica

ALL:	$(MAINS)

$(MAINS):	$(OBJECTS)
	$(CC) $(CFLAGS) -o zic $(OBJECTS) $(LDFLAGS)

zic.o:	$(INC)/stdio.h $(INC)/ctype.h $(INC)/sys/types.h \
	$(INC)/sys/stat.h $(INC)/time.h $(INC)/tzfile.h \
	$(INC)/string.h

scheck.o:	$(INC)/stdio.h $(INC)/ctype.h

ialloc.o:	$(INC)/stdio.h $(INC)/string.h

GLOBALINCS:	$(INC)/stdio.h $(INC)/ctype.h $(INC)/sys/types.h \
	$(INC)/sys/stat.h $(INC)/time.h $(INC)/tzfile.h \
	$(INC)/string.h

clean:
	rm -f $(OBJECTS)

clobber: 
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE) -s INC $(INC)
#bottom#

all: ALL

install: ALL $(FILES)
	-[ -d $(INSLIB)/locale ] || mkdir $(INSLIB)/locale
	$(CH)-chmod 755 $(INSLIB)/locale
	$(CH)-chgrp bin $(INSLIB)/locale
	$(CH)-chown bin $(INSLIB)/locale
	-[ -d $(INSLIB)/locale/TZ ] || mkdir $(INSLIB)/locale/TZ
	$(CH)-chmod 755 $(INSLIB)/locale/TZ
	$(CH)-chgrp bin $(INSLIB)/locale/TZ
	$(CH)-chown bin $(INSLIB)/locale/TZ
	for i in $(FILES); do \
		(install -f $(ROOT)/usr/lib/locale/TZ -m 644 $$i); done
	install -f $(DIR) -u bin -g bin -m 555 $(MAINS)

size:	ALL
	$(SIZE) $(MAINS)

strip:	ALL
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

