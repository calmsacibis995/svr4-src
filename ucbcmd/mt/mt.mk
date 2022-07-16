#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbmt:mt.mk	1.3.1.1"

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

#     Makefile for mt

ROOT =

DIR = $(ROOT)/usr/ucb

INC = $(ROOT)/usr/include

INC1 = $(ROOT)/usr/ucbinclude

LDFLAGS = -s $(SHLIBS) 

CFLAGS = -Dsun -O -I$(INC1) -I. -I$(INC)

STRIP = strip

SIZE = size

#top#

MAKEFILE = mt.mk

MAINS = mt

OBJECTS =  mt.o

SOURCES = mt.c 

ALL:          $(MAINS)

$(MAINS):	mt.o
	$(CC) $(CFLAGS) -o mt mt.o $(LDFLAGS)

mt.o:		$(INC1)/stdio.h \
		$(INC)/ctype.h \
		$(INC1)/sys/types.h \
		$(INC1)/sys/mtio.h \
		$(INC)/sys/ioctl.h \
		$(INC1)/sys/param.h \
		$(INC)/sys/buf.h \
		$(INC1)/sys/file.h \
		$(INC)/sys/conf.h \
		$(INC)/sys/uio.h \
		$(INC)/sys/errno.h


#GLOBALINCS = $(INC)/stdio.h \ 
#	$(INC)/ctype.h \
#	$(INC)/sys/types.h \
#	$(INC)/sys/mtio.h \
#	$(INC)/sys/param.h \
#	$(INC2)/sys/ioctl.h \
#	$(INC)/sys/buf.h \
#	$(INC)/sys/file.h \
#	$(INC)/sys/conf.h \
#	$(INC)/sys/uio.h \
#	$(INC)/sys/errno.h
#
#
clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all :	ALL

install:	ALL
	install -f $(DIR) -m 00555 -u bin -g bin mt 

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

