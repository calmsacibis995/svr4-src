#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbinstall:install.mk	1.2.1.1"


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

#     Makefile for install

ROOT =

DIR = $(ROOT)/usr/ucb

INC = $(ROOT)/usr/include

LDFLAGS = -s $(SHLIBS)

CFLAGS = -O -I$(INC) -DMAXBSIZE=8192

STRIP = strip

SIZE = size

#top#

MAKEFILE = install.mk

MAINS = installcmd

OBJECTS =  install.o

SOURCES = install.c 

ALL:          $(MAINS)

$(MAINS):	install.o
	$(CC) $(CFLAGS) -o install install.o $(LDFLAGS)
	
install.o:	$(INC)/stdio.h $(INC)/sys/param.h \
		$(INC)/sys/stat.h $(INC)/sys/file.h \
		$(INC)/grp.h $(INC)/pwd.h $(INC)/ctype.h \
		$(INC)/errno.h $(INC)/sys/types.h \
		$(INC)/sys/fcntl.h

GLOBALINCS = $(INC)/stdio.h $(INC)/sys/param.h \
		$(INC)/sys/stat.h $(INC)/sys/file.h \
		$(INC)/grp.h $(INC)/pwd.h $(INC)/ctype.h \
		$(INC)/errno.h $(INC)/sys/types.h \
		$(INC)/sys/fcntl.h

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) install

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all :	ALL

install:	ALL
	/etc/install -m 555 -u bin -g bin -f $(DIR) install 

size: ALL
	$(SIZE) install

strip: ALL
	$(STRIP) install

#     These targets are useful but optional

partslist:
	@echo $(MAKEFILE) $(SOURCES) $(LOCALINCS)  |  tr ' ' '\012'  |  sort

productdir:
	@echo $(DIR) | tr ' ' '\012' | sort

product:
	@echo install  |  tr ' ' '\012'  | \
	sed 's;^;$(DIR)/;'

srcaudit:
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) -o $(OBJECTS) install

