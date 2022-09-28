#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)random:random.mk	1.1.1.1"

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft 
#	Corporation and should be treated as Confidential.

#	Makefile for random

ROOT =

#	Where MAINS are to be installed.
INSDIR = $(ROOT)/usr/bin

INS = "install"

INC = $(ROOT)/usr/include

CFLAGS = -O -I$(INC)

#	Common Libraries not found in /lib or /usr/lib.
COMLIB = 

#	Common Libraries and -l<lib> flags.
LDFLAGS = -s $(COMLIB) $(LDLIBS)

STRIP = strip

#top#

MAKEFILE = random.mk


MAINS = random

OBJECTS =  random.o

SOURCES =  random.c

ALL:		$(MAINS)

random:		random.o	
	$(CC) $(CFLAGS)  -o random  random.o   $(LDFLAGS)


random.o:	 $(INC)/stdio.h 

clean:
	rm -f $(OBJECTS)
	
clobber:	
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE) -s INC $(INC)
#bottom#

save:
	cd $(INSDIR); set -x; for m in $(MAINS); do  cp $$m OLD$$m; done

restore:
	cd $(INSDIR); set -x; for m in $(MAINS); do; cp OLD$$m $$m; done

install:	$(MAINS) $(INSDIR)
	$(INS) -f $(INSDIR) -m 0711 -u root -g bin $(MAINS)

strip:
	$(STRIP) $(MAINS)

remove:
	cd $(INSDIR);  rm -f $(MAINS)

$(INSDIR):
	mkdir $(INSDIR);  chmod 755 $(INSDIR);  chown bin $(INSDIR)

partslist:
	@echo $(MAKEFILE) $(LOCALINCS) $(SOURCES)  |  tr ' ' '\012'  |  sort

product:
	@echo $(MAINS)  |  tr ' ' '\012'  | \
		sed -e 's;^;$(INSDIR)/;' -e 's;//*;/;g'

productdir:
	@echo $(INSDIR)

srcaudit:	# will not report missing nor present object or product files.
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) -o $(OBJECTS) $(MAINS)
