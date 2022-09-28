#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)more:more.mk	1.7.3.1"
#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft 
#	Corporation and should be treated as Confidential.


ROOT =

#	Where MAINS are to be installed.
INSDIR = $(ROOT)/usr/bin
LIBDIR	= $(ROOT)/usr/lib

INC = $(ROOT)/usr/include
INS = install

CFLAGS = -O -I$(INC)


#	Common Libraries and -l<lib> flags.
LDFLAGS = -s -lcmd -lcurses -lgen $(SHLIBS)

STRIP = strip

#top#

MAKEFILE = more.mk


MAINS = more

OBJECTS =  more.o

SOURCES =  more.c

DATA = more.help

ALL:		$(MAINS)

more:		more.o	
	$(CC) $(CFLAGS)  -o more  more.o   $(LDFLAGS)


more.o:	 $(INC)/ctype.h $(INC)/signal.h \
	 $(INC)/errno.h $(INC)/sys/types.h \
	 $(INC)/curses.h $(INC)/term.h \
	 $(INC)/sys/ioctl.h $(INC)/setjmp.h \
	 $(INC)/sys/dir.h $(INC)/sys/stat.h \
	 $(INC)/values.h $(INC)/stdlib.h

clean:
	rm -f $(OBJECTS)
	
clobber:	
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE) -s INC $(INC)
#bottom#

save:
	cd $(INSDIR); set -x; for m in $(MAINS); do  cp $$m OLD$$m; done
	cd $(LIBDIR); set -x; for m in $(DATA); do  cp $$m OLD$$m; done

restore:
	cd $(INSDIR); set -x; for m in $(MAINS); do; cp OLD$$m $$m; done
	cd $(LIBDIR); set -x; for m in $(DATA); do; cp OLD$$m $$m; done

install:	$(MAINS) $(INSDIR)
	$(INS) -f $(INSDIR) -m 555 -u bin -g bin $(MAINS)
	rm -f $(INSDIR)/page
	ln $(INSDIR)/$(MAINS) $(INSDIR)/page
	$(INS) -f $(LIBDIR) -m 0644 -u bin -g bin $(DATA)

strip:
	$(STRIP) $(MAINS)

remove:
	cd $(INSDIR);  rm -f $(MAINS)
	cd $(LIBDIR);  rm -f $(DATA)

partslist:
	@echo $(MAKEFILE) $(LOCALINCS) $(SOURCES)  |  tr ' ' '\012'  |  sort

product:
	@echo $(MAINS)  |  tr ' ' '\012'  | \
		sed -e 's;^;$(INSDIR)/;' -e 's;//*;/;g'

productdir:
	@echo $(INSDIR)
